/* Copyright (c) 2007, Stanford University
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*     * Redistributions of source code must retain the above copyright
*       notice, this list of conditions and the following disclaimer.
*     * Redistributions in binary form must reproduce the above copyright
*       notice, this list of conditions and the following disclaimer in the
*       documentation and/or other materials provided with the distribution.
*     * Neither the name of Stanford University nor the
*       names of its contributors may be used to endorse or promote products
*       derived from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY STANFORD UNIVERSITY ``AS IS'' AND ANY
* EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL STANFORD UNIVERSITY BE LIABLE FOR ANY
* DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
* ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/ 

#include <stddef.h>
#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>
#include <assert.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>
#include <pthread.h>
#include "stddefines.h"

#define UNIT_SIZE (1024 * 5)      // approx 5kB per html file
#define START_ARRAY_SIZE 2000

enum {
   START,
   IN_TAG,
   IN_ATAG,
   FOUND_HREF,
   START_LINK
};

typedef struct link_elem {
   char *filename;
   struct link_elem *next;
} link_elem_t;


typedef struct link_head {
   char *link;
   struct link_elem *elem;
} link_head_t;


typedef struct flist{
   char *data;
   char *name;
   int fd;
   off_t size;
   struct flist *next;
} filelist_t;

typedef struct {
   filelist_t *filelist;
   filelist_t *currfile;
   int num_files;
   int unit_size;
} ri_data_t;

typedef struct
{
   int length1;
   int length2;
   int length_out_pos;
   link_head_t *data1;
   link_head_t *data2;
   link_head_t *out;
} merge_data_t;

filelist_t *filelist;
filelist_t *currfile;
int num_files;
int data_size;

link_head_t **links;
int *use_len;
int *length;

int req_data;

pthread_mutex_t file_lock;

/** cleanup()
 *  
 */
void cleanup() {
   filelist_t *curr_ptr = filelist;
   filelist_t *prev_ptr;
   
   while(curr_ptr != NULL) {
      /*munmap(curr_ptr->data, curr_ptr->size + 1);
      close(curr_ptr->fd);*/
      free(curr_ptr->data);
      free(curr_ptr->name);

      prev_ptr = curr_ptr;
      curr_ptr = curr_ptr->next;
      free(prev_ptr);   
   }
}

/** addtolist()
 *  Create and update the list from links to files
 */
void addtolist(filelist_t *file) {
   
   file->next = NULL;
   
   if (filelist == NULL) {
      filelist = file;
      currfile = file;
   }
   else {
      currfile->next = file;
      currfile = file;
   }
   
   num_files++;
}

/** recursedirs()
 *  Recursively go througn the directories with the HTML files and memory
 *	map them
 */
void recursedirs(char *name) {
   
   DIR *dp;
   struct dirent *ep;
   struct stat finfo;
   char *path;
   
   if (((req_data > 0) && (data_size < req_data)) || (req_data <= 0)) {
      dp = opendir (name);
      if (dp != NULL)      // it's a directory
      {
         while ((ep = readdir (dp)) != NULL) {
            if (strcmp(ep->d_name, ".") == 0) continue;
            if (strcmp(ep->d_name, "..") == 0) continue;
            
            path = (char *)malloc(strlen(ep->d_name) + strlen(name) + 2);
            sprintf(path, "%s/%s",name,ep->d_name);
            
            recursedirs(path);
            free(path);
         }
         (void) closedir (dp);
      }
      else  // it's not a directory, mmap file and add it to the list
      {
         filelist_t *file;
         CHECK_ERROR((file = (filelist_t *)malloc(sizeof(filelist_t))) == NULL);
         file->fd = open(name, O_RDONLY);
         if (file->fd < 0) {
            free(file);   
         }
         else {
            int ret = fstat(file->fd, &finfo);
            if (ret < 0) {
               free(file);
            }
            else {
               file->size = finfo.st_size;
               char *f_data = mmap(0, file->size + 1, PROT_READ | PROT_WRITE, 
                                                       MAP_PRIVATE, file->fd, 0);
               if (f_data == NULL) {
                  free(file);
               }
               else {
                  CHECK_ERROR((file->name = (char *)malloc(strlen(name) + 1)) == NULL);
                  data_size += finfo.st_size;
                  strcpy(file->name, name);
                  file->data = (char *)malloc(finfo.st_size + 1);
                  memcpy(file->data, f_data, finfo.st_size);
                  addtolist(file);
                  munmap(f_data, file->size + 1);
                  close(file->fd);
               }
            }
         }
      }
   }
   return;
}

/** dobsearch()
 *  Looking for a specific link
 */
int dobsearch(char* link, link_head_t *arr, int curr_use_len)
{
   int cmp, high = curr_use_len, low = -1, next;

   // Binary search the array to find the key
   while (high - low > 1)
   {
       next = (high + low) / 2;   
       cmp = strcmp(link, arr[next].link);
       if (cmp == 0)
          return next;
       else if (cmp < 0)
           high = next;
       else
           low = next;
   }

   return high;
}

/** insert_sorted()
 *  Look at the current links and insert into the lists in a sorted manner
 */
// modified to use hand-over-hand locking
void insert_sorted(char * link, char *filename, int curr_thread) {
   
   dprintf("%d: Inserting %s.... ", curr_thread, link);
   int curr_use_len = use_len[curr_thread];
   int curr_length = length[curr_thread];
   link_head_t *arr = links[curr_thread];
   
   int pos = dobsearch(link, arr, curr_use_len);
   
   if (pos >= curr_use_len)
   {
      // at end
      dprintf("%d: Inserting at end, %d, %d, %d\n", curr_thread, curr_use_len, curr_length, pos);
      arr[curr_use_len].link = link;
	   link_elem_t *new_elem;
      CHECK_ERROR((new_elem = (link_elem_t *)malloc(sizeof(link_elem_t))) == NULL);
      new_elem->next = NULL;
      new_elem->filename = filename; 
      arr[curr_use_len].elem = new_elem;

  	   (use_len[curr_thread])++;
	}
   else if (pos < 0)
   {
      // at front
      dprintf("%d: Inserting at front %d, %d, %d\n", curr_thread, curr_use_len, curr_length, pos);
      memmove(&arr[1], &arr[0], curr_use_len*sizeof(link_head_t));
      arr[0].link = link;
      
      link_elem_t *new_elem;
      
      CHECK_ERROR((new_elem = (link_elem_t *)malloc(sizeof(link_elem_t))) == NULL);
      new_elem->next = NULL;
      new_elem->filename = filename; 
      arr[0].elem = new_elem;
      
	   (use_len[curr_thread])++;
   }
   else if (strcmp(link, arr[pos].link) == 0)
   {
      // match
      dprintf("%d: link exists %d, %d, %d\n", curr_thread, curr_use_len, curr_length, pos);
      assert(arr[pos].elem);
      link_elem_t *new_elem;
      CHECK_ERROR((new_elem = (link_elem_t *)malloc(sizeof(link_elem_t))) == NULL);
      new_elem->filename = filename;
      new_elem->next = arr[pos].elem;
      arr[pos].elem = new_elem;
	}
   else
   {
      // insert at pos
      dprintf("%d: Inserting in middle %d, %d, %d\n", curr_thread, curr_use_len, curr_length, pos);
      memmove(&arr[pos+1], &arr[pos], (curr_use_len-pos)*sizeof(link_head_t));
      arr[pos].link = link;
	   
	   link_elem_t *new_elem;
      CHECK_ERROR((new_elem = (link_elem_t *)malloc(sizeof(link_elem_t))) == NULL);
      new_elem->next = NULL;
      new_elem->filename = filename; 
      arr[pos].elem = new_elem;
      
	   (use_len[curr_thread])++;
   }

	if(use_len[curr_thread] == curr_length)
	{
		length[curr_thread] *= 2;
		dprintf("%d: Increasing size of array at %p to %d\n", curr_thread, links[curr_thread], length[curr_thread]); 
	   links[curr_thread] = (link_head_t*)REALLOC(links[curr_thread],
	                                          length[curr_thread]*sizeof(link_head_t));
	   dprintf("%d: Realloced array at %p of size %d\n", curr_thread, links[curr_thread], length[curr_thread]); 
	}
   
   
   dprintf("%d: Inserted\n", curr_thread);    
}

/** getlinks()
 *  Get the links in the files and update the relevant lists
 */
void *getlinks(void *arg) 
{
   off_t j;
   char *link_end;
   int state = START;
   int curr_thread = (int) arg;
   
   filelist_t *file = NULL;
   
   pthread_mutex_lock(&file_lock);
   file = currfile;
   if (currfile != NULL) currfile = currfile->next;
   pthread_mutex_unlock(&file_lock);
   
   // go through each file and look for links.
   //for (i=0; i<t_arg->num_files; i++) 
   while(file != NULL)
   {
      assert(file);
      dprintf("Looking at file %s, size %d\n",file->name, (int)file->size);
      
      for (j=0; j<file->size; j++) 
      {
         switch (state)
         {
            case START:
               if (file->data[j] == '<') state = IN_TAG;
               break;
               
            case IN_TAG:
               //dprintf("Found a tag\n");
               if (file->data[j] == 'a') state = IN_ATAG;
               else if (file->data[j] == ' ') state = IN_TAG;
               else state = START;               
               break;
         
            case IN_ATAG:
               //dprintf("Found <a\n");
               if (file->data[j] == 'h')
               {
                  if (strncmp(&file->data[j], "href", 4) == 0) 
                  {
                     state = FOUND_HREF;   
                     j += 3;
                  }
                  else state = START;
               }
               else if (file->data[j] == ' ') state = IN_ATAG;
               else state = START;
               break;
               
            case FOUND_HREF:
               //dprintf("Found href\n");
               if (file->data[j] == ' ') state = FOUND_HREF;
               else if (file->data[j] == '=') state = FOUND_HREF;
               else if (file->data[j] == '\"') state  = START_LINK;
               else state = START;
               break;
            
            case START_LINK:
               //dprintf("Found a link\n");
               link_end = NULL;
               link_end = strchr(&(file->data[j]), '\"');
               if (link_end != NULL)
               {
                  link_end[0] = 0;
                  insert_sorted(&(file->data[j]), file->name, curr_thread);       
                  //dprintf("Found key %s in file %s\n", &(file->data[j]), file->name);
                  
                  j += strlen(&(file->data[j]));
               }
               state = START;
               break;
         }
      }
            
      pthread_mutex_lock(&file_lock);
      file = currfile;
      if (currfile != NULL) currfile = currfile->next;
      pthread_mutex_unlock(&file_lock);
   }
   
   return (void *)0;
}

/** merge_sections()
 *  Addup the partial lists to get the final lists
 */
void *merge_sections(void *args_in)  
{
   merge_data_t* args = (merge_data_t*)args_in;
   int cmp_ret;
   int curr1, curr2;
   int length_out = 0;
   
   for (curr1 = 0, curr2 = 0; 
        curr1 < args->length1 && curr2 < args->length2;) 
   {
      cmp_ret = strcmp(args->data1[curr1].link, args->data2[curr2].link);
      if (cmp_ret == 0)
      {
        //add files from second list to first list 
        link_elem_t *curr_elem = args->data2[curr2].elem;
        link_elem_t *prev_elem = NULL;
        while (curr_elem != NULL)   {
            prev_elem = curr_elem; 
            curr_elem = curr_elem->next;   
        }
        assert(prev_elem);
        prev_elem->next = args->data1[curr1].elem;
        args->data1[curr1].elem = args->data2[curr2].elem;
        curr2++;
      }
      else if (cmp_ret < 0)
      {
         // insert args->data1[curr1].word
         memcpy(&args->out[length_out], &args->data1[curr1], sizeof(link_head_t));
         length_out++;
         curr1++;
      }
      else
      {
         // insert args->data2[curr2].word
         memcpy(&args->out[length_out], &args->data2[curr2], sizeof(link_head_t));
         length_out++;
         curr2++;
      }
   }
   
   // copy the remaining elements
   memcpy(&args->out[length_out], &args->data1[curr1], (args->length1 - curr1)*sizeof(link_head_t));
   memcpy(&args->out[length_out], &args->data2[curr2], (args->length2 - curr2)*sizeof(link_head_t));

   // set length
   length_out += (args->length1 - curr1) + (args->length2 - curr2);
   length[args->length_out_pos] = length_out;
   free(args->data1);
   free(args->data2);
   free(args);
   
   return (void*)0;
}



int main(int argc, char **argv)
{  
   int num_procs;
   pthread_t *pid;
   pthread_attr_t attr;
   int i;
   //int num_per_thread, excess;     
   if (argc != 2) {
      fprintf(stderr, "Usage: %s <start directory>\n",argv[0]); 
      exit(-1);  
   }   
   CHECK_ERROR(argv[1] == NULL);
   
   //ri_data.filelist = NULL;
   //ri_data.currfile = NULL;
   //ri_data.num_files = 0;
   //ri_data.unit_size = UNIT_SIZE;
   
   char *req_data_str = getenv("RI_DATASIZE");
   if (req_data_str != NULL)
      req_data = atoi(req_data_str);
   else
      req_data = 0;
   
   printf("Reqd data = %d\n", req_data);
   
   recursedirs(argv[1]);
   
   printf("Number of files added = %d, total size = %d\n", num_files, data_size);
   dprintf("Files are:\n");
   currfile = filelist;
   while (currfile!= NULL) {
      dprintf("%s\n", currfile->name);
      currfile = currfile->next;
   }
   
   pthread_attr_init(&attr);
   pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);
   CHECK_ERROR((num_procs = sysconf(_SC_NPROCESSORS_ONLN)) <= 0);
   
   CHECK_ERROR( (pid = (pthread_t *)malloc(sizeof(pthread_t) * num_procs)) == NULL);
   pthread_mutex_init(&file_lock, NULL);

   printf("Running on %d procs\n", num_procs);

   currfile = filelist;
   
   links = (link_head_t **)MALLOC(num_procs * sizeof(link_head_t *));
   use_len = (int *)CALLOC(num_procs, sizeof(int));
   length = (int *)MALLOC(num_procs * sizeof(int));

   for (i = 0; i < num_procs; i++) {
      links[i] = (link_head_t *)CALLOC(START_ARRAY_SIZE, sizeof(link_head_t));
      length[i] = START_ARRAY_SIZE;
      CHECK_ERROR((pthread_create(&(pid[i]), &attr, getlinks, (void *)i)) != 0);
   }

   for (i=0; i<num_procs; i++) {
      pthread_join(pid[i], NULL);
   }
   
   
   
   dprintf(stderr, "Done with getting links, now merging\n");
   
   // Join the arrays
   int num_threads = num_procs / 2;
   int rem_num = num_procs % 2;
   link_head_t **final = (link_head_t**)MALLOC(num_procs*sizeof(link_head_t*));
   
   while(num_threads > 0)
   {
      dprintf(stderr, "Merging with %d threads, rem = %d\n", num_threads, rem_num);
	   for(i=0; i<num_threads; i++)
	   {
		   merge_data_t *m_args = (merge_data_t*)malloc(sizeof(merge_data_t));
		   m_args->length1 = use_len[i*2];
         m_args->length2 = use_len[i*2 + 1];
         m_args->length_out_pos = i;
         m_args->data1 = links[i*2];
         m_args->data2 = links[i*2 + 1];
         int tlen = m_args->length1 + m_args->length2;
         final[i] = (link_head_t *)malloc(tlen*sizeof(link_head_t));
         m_args->out = final[i];
         
		   CHECK_ERROR(pthread_create(&pid[i], &attr, merge_sections, (void*)m_args) != 0);
	   }

	   /* Barrier, wait for all threads to finish */
	   for (i = 0; i < num_threads; i++)
	   {
		  int ret_val;
		  CHECK_ERROR(pthread_join(pid[i], (void **)(void*)&ret_val) != 0);
		  CHECK_ERROR(ret_val != 0);

		  links[i] = final[i];
		  use_len[i] = length[i];
	   }
	   
      if (rem_num == 1)
      {
         links[num_threads] = links[num_threads*2];
         use_len[num_threads] = use_len[num_threads*2];
      }
      
      int old_num = num_threads;
	   num_threads = (num_threads+rem_num) / 2;
      rem_num = (old_num+rem_num) % 2;
   }


   link_elem_t *curr_elem, *prev_elem = NULL;
   int j;
   for (j = 0; j < use_len[0]; j++)
   {
      dprintf("\nLink to \"%s\" found in:\n", links[0][j].link);
      curr_elem = links[0][j].elem;
      while(curr_elem != NULL)
      {
         dprintf("\t%s\n",curr_elem->filename);
         prev_elem = curr_elem;
         curr_elem = curr_elem->next;
         prev_elem->filename = "DEAD";   
         free(prev_elem);
      }
   }
   
   free(links[0]);

   
   free(links);
   free(final);
   free(use_len);
   free(length);
   cleanup();
   pthread_attr_destroy(&attr);
   free(pid);
   
   return 0;
}
