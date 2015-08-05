/* Copyright (C) 2002, 2004, 2007 Christopher Clark, Princeton University */
/* Parallelization by Christian Bienia */

#ifndef _HASHTABLE_PRIVATE_H_
#define _HASHTABLE_PRIVATE_H_

#include <errno.h>
#include "config.h"

#ifdef ENABLE_PTHREADS
#include <pthread.h>
#endif

#include "hashtable.h"


/*****************************************************************************/
#ifndef STRUCT_ENTRY
#define STRUCT_ENTRY 1
struct hash_entry
{
    void *k, *v;
    unsigned int h;
  struct hash_entry *next;
};
#endif

struct hashtable {
    unsigned int tablelength;
    struct hash_entry **table;
#ifdef ENABLE_PTHREADS
    //Each entry in table array is protected with its own lock
    pthread_mutex_t *locks;
#endif
#ifdef ENABLE_DYNAMIC_EXPANSION
    unsigned int entrycount;
    unsigned int loadlimit;
    unsigned int primeindex;
#endif
    unsigned int (*hashfn) (void *k);
    int (*eqfn) (void *k1, void *k2);
    int free_keys;
};

/*****************************************************************************/
unsigned int
hash(struct hashtable *h, void *k);

/*****************************************************************************/
/* indexFor */
static inline unsigned int
indexFor(unsigned int tablelength, unsigned int hashvalue) {
    return (hashvalue % tablelength);
};

/* Only works if tablelength == 2^N */
/*static inline unsigned int
indexFor(unsigned int tablelength, unsigned int hashvalue)
{
    return (hashvalue & (tablelength - 1u));
}
*/

/*****************************************************************************/
#define freekey(X) free(X)
/*define freekey(X) ; */


/*****************************************************************************/

#endif /* _HASHTABLE_PRIVATE_H_ */

/*
 * Copyright (c) 2002, 2007 Christopher Clark and Princeton University
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
