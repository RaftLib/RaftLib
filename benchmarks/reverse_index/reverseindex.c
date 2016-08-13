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
#include "stddefines.h"
#include "MapReduceScheduler.h"

#define UNIT_SIZE (1024 * 5)      // approx 5kB per html file

enum {
   START,
   IN_TAG,
   IN_ATAG,
   FOUND_HREF,
   START_LINK
};

/* The list of files */
typedef struct flist{
   char *data;
   char *name;
   int fd;
   off_t size;
   struct flist *next;
} filelist_t;

typedef struct {
   char **vals;
   int length;   
} vals_t;

typedef struct {
   filelist_t *filelist;
   filelist_t *currfile;
   int num_files;
   int unit_size;
} ri_data_t;

ri_data_t ri_data;
int data_size;
int req_data;

/** cleanup()
 *  
 */
void cleanup() {
   filelist_t *curr_ptr = ri_data.filelist;
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
   
   if (ri_data.filelist == NULL) {
      ri_data.filelist = file;
      ri_data.currfile = file;
   }
   else {
      ri_data.currfile->next = file;
      ri_data.currfile = file;
   }
   
   ri_data.num_files++;
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

/** mystrcmp()
 *  String comparison function for keys
 */
int mystrcmp(const void *s1, const void *s2)
{
   return strcmp((const char *)s1, (const char *) s2);
}


/** revidx_splitter()
 *  Allocate several files to each map task (until we run out of files).
 */
int revidx_splitter(void *data_in, int req_units, map_args_t *out)
{
   ri_data_t *ri_data = (ri_data_t *)data_in; 
   int i;
   
   assert(data_in);
   assert(out);
   
   assert(ri_data->num_files >= 0);
   assert(ri_data->filelist);
   assert(req_units);
   
   // End of filelist reached, return FALSE for no more data
   if (ri_data->currfile == NULL) return 0;

   // Set the start of the next data
   out->data = (void *)ri_data->currfile;
   
   out->length = 0;
   
   for (i=0; i<req_units; i++) 
   {
      out->length++;
      ri_data->currfile = ri_data->currfile->next;
      if (ri_data->currfile == NULL) return 1;
   }

   // Return true since the out data is valid
   return 1;
}


/** revidx_map()
 *  Map task that goes through each file and finds the links
 *	and updates the relevant lists
 */
void revidx_map(map_args_t *args) 
{
   
   assert(args);
   int i;
   off_t j;
   char *link_end;
   int state = START;
   
   filelist_t *file = (filelist_t *)args->data;
   
   
   // go through each file and look for links.
   for (i=0; i<args->length; i++) 
   {
      assert(file);
      //dprintf("Looking at file %s, size %d\n",file->name, file->size);
      
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
                  emit_intermediate((void *)&(file->data[j]),        // key
                                    (void *)file->name,              // val 
                                    strlen(&(file->data[j]) + 1));   // size
                  
                  //dprintf("Found key %s in file %s\n", &(file->data[j]), file->name);
                  
                  j += strlen(&(file->data[j]));
               }
               state = START;
               break;
         }
      }
            
      file = file->next;
   }
}


/** revidx_reduce()
 * Reduce task that combines the partial sums for each link
 */
void revidx_reduce(void *key_in, void **vals_in, int vals_len) 
{
   char *key = (char *)key_in;
   char **val_arr = (char **)vals_in;
   int i;
   
   assert(key);
   assert(val_arr);
   
   vals_t *vals;
   CHECK_ERROR((vals = (vals_t *)malloc(sizeof(vals_t))) == NULL);
   
   CHECK_ERROR((vals->vals = (char **)malloc(sizeof(char *) * vals_len)) == NULL);
   for (i = 0; i < vals_len; i++) {
      vals->vals[i] = val_arr[i];
   }
   vals->length = vals_len;
   
   emit(key_in, (void *)vals);
}

     
int main (int argc, char **argv)
{
   final_data_t ri_vals;
   int i, j;
   
   if (argc != 2) {
      fprintf(stderr, "Usage: %s <start directory>\n",argv[0]); 
      exit(-1);  
   }   
   CHECK_ERROR(argv[1] == NULL);

   ri_data.filelist = NULL;
   ri_data.currfile = NULL;
   ri_data.num_files = 0;
   ri_data.unit_size = UNIT_SIZE;
   data_size = 0;
   
   char *req_data_str = getenv("RI_DATASIZE");
   if (req_data_str != NULL)
      req_data = atoi(req_data_str);
   else
      req_data = 0;
   
   printf("Reqd data = %d\n", req_data);
   
   recursedirs(argv[1]);
   
   printf("Number of files added = %d, total size = %d\n", ri_data.num_files, data_size);
   dprintf("Files are:\n");
   ri_data.currfile = ri_data.filelist;
   while (ri_data.currfile!= NULL) {
      dprintf("%s\n", ri_data.currfile->name);
      ri_data.currfile = ri_data.currfile->next;
   }
   
   // set currfile to the start of the list before calling mapreduce so that
   // the splitter starts from the first file.
   ri_data.currfile = ri_data.filelist;   
   
   // Setup scheduler args
   scheduler_args_t sched_args;
   memset(&sched_args, 0, sizeof(scheduler_args_t));
   sched_args.task_data = &ri_data;
   sched_args.map = revidx_map;
   sched_args.reduce = revidx_reduce;
   sched_args.splitter = revidx_splitter;
   sched_args.key_cmp = mystrcmp;
   sched_args.unit_size = ri_data.unit_size;
   sched_args.partition = NULL; // use default
   sched_args.result = &ri_vals;
   sched_args.data_size = data_size;
   sched_args.L1_cache_size = atoi(GETENV("MR_L1CACHESIZE")); //1024 * 64;
   sched_args.num_map_threads = atoi(GETENV("MR_NUMTHREADS"));//8;
   sched_args.num_reduce_threads = atoi(GETENV("MR_NUMTHREADS"));//16;
   sched_args.num_merge_threads = atoi(GETENV("MR_NUMTHREADS")) / 2;//8;
   sched_args.num_procs = atoi(GETENV("MR_NUMPROCS"));//16;
   sched_args.key_match_factor = (float)atof(GETENV("MR_KEYMATCHFACTOR"));//2;

   dprintf("\n\n");
   
   printf("ReverseIndex: Calling MapReduce Scheduler\n");

   CHECK_ERROR(map_reduce_scheduler(&sched_args) < 0);

   printf("ReverseIndex: MapReduce Completed\n");  

   for (i = 0; i < ri_vals.length; i++) {
      keyval_t *keyval = (keyval_t *)&(ri_vals.data[i]);
      dprintf("\nLink to \"%s\" found in:\n", (char *)keyval->key);
      vals_t *vals = (vals_t *)keyval->val;
      for (j = 0; j < vals->length; j++) 
      {
         dprintf("\t%s\n", vals->vals[j]);
      }
      free(vals->vals);
      free(vals);
   }

   free(ri_vals.data);
   
   cleanup();
   return 0;
}


