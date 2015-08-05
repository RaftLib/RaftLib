/* Copyright (C) 2004, 2007 Christopher Clark, Princeton University */
/* Parallelization by Christian Bienia */

/*
 * Copyright (c) 2002, 2007 Christopher Clark, Princeton University
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 
 * * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 * 
 * * Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 * 
 * * Neither the name of the original author; nor the names of any contributors
 * may be used to endorse or promote products derived from this software
 * without specific prior written permission.
 * 
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER
 * OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#ifdef ENABLE_DMALLOC
#include <dmalloc.h>
#endif //ENABLE_DMALLOC

#include "hashtable.h"
#include "hashtable_private.h"

/*
Credit for primes table: Aaron Krowne
 http://br.endernet.org/~akrowne/
 http://planetmath.org/encyclopedia/GoodHashTablePrimes.html
*/
static const unsigned int primes[] = {
53, 97, 193, 389,
769, 1543, 3079, 6151,
12289, 24593, 49157, 98317,
196613, 393241, 786433, 1572869,
3145739, 6291469, 12582917, 25165843,
50331653, 100663319, 201326611, 402653189,
805306457, 1610612741
};
const unsigned int prime_table_length = sizeof(primes)/sizeof(primes[0]);
#ifdef ENABLE_DYNAMIC_EXPANSION
const float max_load_factor = 0.65;
#endif

/*****************************************************************************/
struct hashtable * hashtable_create(unsigned int minsize,
                                    unsigned int (*hashf) (void*),
                                    int (*eqf) (void*,void*),
                                    int free_keys) {
  struct hashtable *h;
  unsigned int pindex, size = primes[0];

  /* Check requested hashtable isn't too large */
  if (minsize > (1u << 30)) return NULL;
  /* Enforce size as prime */
  for (pindex=0; pindex < prime_table_length; pindex++) {
    if (primes[pindex] > minsize) { size = primes[pindex]; break; }
  }
  h = (struct hashtable *)malloc(sizeof(struct hashtable));
  if (NULL == h) return NULL; /*oom*/
  h->table = (struct hash_entry **)malloc(sizeof(struct hash_entry*) * size);
  if (NULL == h->table) { free(h); return NULL; } /*oom*/
  memset(h->table, 0, size * sizeof(struct hash_entry *));
  h->tablelength  = size;
#ifdef ENABLE_PTHREADS
  //allocate and initialize array with locks
  h->locks = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t) * size);
  if(NULL == h->locks) {free(h->table); free(h); return NULL;} /*oom*/
  for(pindex=0; pindex<size; pindex++) {
    pthread_mutex_init(&(h->locks[pindex]), NULL);
  }
#endif
#ifdef ENABLE_DYNAMIC_EXPANSION
  h->primeindex   = pindex;
  h->entrycount   = 0;
  h->loadlimit    = (unsigned int) ceil(size * max_load_factor);
#endif
  h->hashfn       = hashf;
  h->eqfn         = eqf;
  h->free_keys    = free_keys;
  return h;
}

/*****************************************************************************/
unsigned int hash(struct hashtable *h, void *k) {
  /* Aim to protect against poor hash functions by adding logic here
   * - logic taken from java 1.4 hashtable source */
  unsigned int i = h->hashfn(k);
  i += ~(i << 9);
  i ^=  ((i >> 14) | (i << 18)); /* >>> */
  i +=  (i << 4);
  i ^=  ((i >> 10) | (i << 22)); /* >>> */
  return i;
}

#ifdef ENABLE_PTHREADS
/*****************************************************************************/
pthread_mutex_t * hashtable_getlock(struct hashtable *h, void *k) {
  unsigned int hashvalue, index;

  hashvalue = hash(h,k);
  //NOTE: If dynamic expansion is disabled then tablelength is read-only
  index = indexFor(h->tablelength,hashvalue);
  return &(h->locks[index]);
}
#endif

#ifdef ENABLE_DYNAMIC_EXPANSION
/*****************************************************************************/
static int hashtable_expand(struct hashtable *h) {
  /* Double the size of the table to accomodate more entries */
  struct hash_entry **newtable;
  struct hash_entry *e;
  struct hash_entry **pE;
  unsigned int newsize, i, index;
  /* Check we're not hitting max capacity */
  if (h->primeindex == (prime_table_length - 1)) return 0;
  newsize = primes[++(h->primeindex)];

  newtable = (struct hash_entry **)malloc(sizeof(struct hash_entry*) * newsize);
  if (NULL != newtable) {
    memset(newtable, 0, newsize * sizeof(struct hash_entry *));
    /* This algorithm is not 'stable'. ie. it reverses the list
     * when it transfers entries between the tables */
    for (i = 0; i < h->tablelength; i++) {
      while (NULL != (e = h->table[i])) {
        h->table[i] = e->next;
        index = indexFor(newsize,e->h);
        e->next = newtable[index];
        newtable[index] = e;
      }
    }
    free(h->table);
    h->table = newtable;
  } else {
    /* Plan B: realloc instead */
    newtable = (struct hash_entry **) realloc(h->table, newsize * sizeof(struct hash_entry *));
    if (NULL == newtable) { (h->primeindex)--; return 0; }
    h->table = newtable;
    memset(newtable[h->tablelength], 0, newsize - h->tablelength);
    for (i = 0; i < h->tablelength; i++) {
      for (pE = &(newtable[i]), e = *pE; e != NULL; e = *pE) {
        index = indexFor(newsize,e->h);
        if (index == i) {
          pE = &(e->next);
        } else {
          *pE = e->next;
          e->next = newtable[index];
          newtable[index] = e;
        }
      }
    }
  }
  h->tablelength = newsize;
  h->loadlimit   = (unsigned int) ceil(newsize * max_load_factor);

  return -1;
}
#endif //ENABLE_DYNAMIC_EXPANSION

#ifdef ENABLE_DYNAMIC_EXPANSION
//NOTE: Do not count entries in parallel version b/c it requires global writes
/*****************************************************************************/
unsigned int hashtable_count(struct hashtable *h) {
  return h->entrycount;
}
#endif //ENABLE_DYNAMIC_EXPANSION

/*****************************************************************************/
int hashtable_insert(struct hashtable *h, void *k, void *v) {
  /* This method allows duplicate keys - but they shouldn't be used */
  unsigned int index;
  struct hash_entry *e;
#ifdef ENABLE_DYNAMIC_EXPANSION
  if (++(h->entrycount) > h->loadlimit) {
    /* Ignore the return value. If expand fails, we should
     * still try cramming just this value into the existing table
     * -- we may not have memory for a larger table, but one more
     * element may be ok. Next time we insert, we'll try expanding again.*/
    hashtable_expand(h);
  }
#endif

  e = (struct hash_entry *)malloc(sizeof(struct hash_entry));
  if (NULL == e) { /*oom*/
#ifdef ENABLE_DYNAMIC_EXPANSION
      --(h->entrycount);
#endif
      return 0;
  }
  e->h = hash(h,k);
  //NOTE: If dynamic expansion is disabled then tablelength is read-only
  index = indexFor(h->tablelength,e->h);
  e->k = k;
  e->v = v;
  //If parallelization is enabled we need the lock for entry table[index] for that
  e->next = h->table[index];
  h->table[index] = e;
  return -1;
}

/*****************************************************************************/
/* returns value associated with key */
void * hashtable_search(struct hashtable *h, void *k) {
  struct hash_entry *e;
  unsigned int hashvalue, index;
  hashvalue = hash(h,k);

  index = indexFor(h->tablelength,hashvalue);
  //If parallelization is enabled we need the lock for entry table[index] for that
  e = h->table[index];
  while (NULL != e) {
    /* Check hash value to short circuit heavier comparison */
    if ((hashvalue == e->h) && (h->eqfn(k, e->k))) return e->v;
    e = e->next;
  }

  return NULL;
}

void * hashtable_change(struct hash_entry * e, void * newv) {
  //Locking required
  e->v = newv;

  return NULL;
}

/*****************************************************************************/
/* returns value associated with key */
void * hashtable_remove(struct hashtable *h, void *k) {
  /* TODO: consider compacting the table when the load factor drops enough,
   *       or provide a 'compact' method. */

  struct hash_entry *e;
  struct hash_entry **pE;
  void *v;
  unsigned int hashvalue, index;

  hashvalue = hash(h,k);
  //NOTE: tablelength is read-only if dynamic expansion is disabled
  index = indexFor(h->tablelength,hash(h,k));
  //Proper locking required
  pE = &(h->table[index]);
  e = *pE;
  while (NULL != e) {
    /* Check hash value to short circuit heavier comparison */
    if ((hashvalue == e->h) && (h->eqfn(k, e->k))) {
      *pE = e->next;
#ifdef ENABLE_DYNAMIC_EXPANSION
      h->entrycount--;
#endif
      v = e->v;
      if(h->free_keys) freekey(e->k);
      free(e);
      return v;
    }
    pE = &(e->next);
    e = e->next;
  }
  return NULL;
}

/*****************************************************************************/
/* destroy */
void hashtable_destroy(struct hashtable *h, int free_values) {
  unsigned int i;
  struct hash_entry *e, *f;
  struct hash_entry **table = h->table;

  if (free_values) {
    for (i = 0; i < h->tablelength; i++) {
      e = table[i];
      while (NULL != e) { f = e; e = e->next; if(h->free_keys) freekey(f->k); free(f->v); free(f); }
    }
  } else {
    for (i = 0; i < h->tablelength; i++) {
      e = table[i];
      while (NULL != e) { f = e; e = e->next; if(h->free_keys) freekey(f->k); free(f); }
    }
  }
#ifdef ENABLE_PTHREADS
  for(i=0; i<h->tablelength; i++) {
    pthread_mutex_destroy(&(h->locks[i]));
  }
  free(h->locks);
#endif
  free(h->table);
  free(h);
}

