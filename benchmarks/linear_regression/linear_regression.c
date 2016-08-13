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

#include <stdio.h>
#include <strings.h>
#include <string.h>
#include <stddef.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>

#include "MapReduceScheduler.h"
#include "stddefines.h"


typedef struct {
   char x;
   char y;
} POINT_T;

enum {
   KEY_SX = 0,
   KEY_SY,
   KEY_SXX,
   KEY_SYY,
   KEY_SXY,
};

static int intkeycmp(const void *v1, const void *v2)
{
   int i1 = (int)v1;
   int i2 = (int)v2;

   if (i1 < i2) 
       return 1;
   else if (i1 > i2) 
       return -1;
   else 
       return 0;
}

/** sort_map()
 *  Sorts based on the val output of wordcount
 */
static void linear_regression_map(map_args_t *args) 
{
   assert(args);
   
   POINT_T *data = (POINT_T *)args->data;
   int i;

   assert(data);

   long long * SX  = CALLOC(sizeof(long long), 1);
   long long * SXX = CALLOC(sizeof(long long), 1);
   long long * SY  = CALLOC(sizeof(long long), 1);
   long long * SYY = CALLOC(sizeof(long long), 1);
   long long * SXY = CALLOC(sizeof(long long), 1);


   for (i = 0; i < args->length; i++)
   {
      //Compute SX, SY, SYY, SXX, SXY
      *SX  += data[i].x;
      *SXX += data[i].x*data[i].x;
      *SY  += data[i].y;
      *SYY += data[i].y*data[i].y;
      *SXY += data[i].x*data[i].y;
   }

   emit_intermediate((void*)KEY_SX,  (void*)SX,  sizeof(void*)); 
   emit_intermediate((void*)KEY_SXX, (void*)SXX, sizeof(void*)); 
   emit_intermediate((void*)KEY_SY,  (void*)SY,  sizeof(void*)); 
   emit_intermediate((void*)KEY_SYY, (void*)SYY, sizeof(void*)); 
   emit_intermediate((void*)KEY_SXY, (void*)SXY, sizeof(void*)); 
}

static int linear_regression_partition(int reduce_tasks, void* key, int key_size)
{
   return default_partition(reduce_tasks, (void *)&key, key_size);
}

/** linear_regression_reduce()
 *
 */
static void linear_regression_reduce(void *key_in, void **vals_in, int vals_len) 
{
   long long **vals = (long long **)vals_in;
   long long * sum = CALLOC(sizeof(long long), 1);
   int i;

   assert(vals);

   for (i = 0; i < vals_len; i++)
   {
      *sum += *vals[i];
      free(vals[i]);
   }

   emit(key_in, (void *)sum);
}

int main(int argc, char *argv[]) {

   final_data_t final_vals;
   int fd;
   char * fdata;
   char * fname;
   struct stat finfo;
   int i;

   struct timeval starttime,endtime;

   // Make sure a filename is specified
   if (argv[1] == NULL)
   {
      printf("USAGE: %s <filename>\n", argv[0]);
      exit(1);
   }
   
   fname = argv[1];

   printf("Linear Regression: Running...\n");
   
   // Read in the file
   CHECK_ERROR((fd = open(fname, O_RDONLY)) < 0);
   // Get the file info (for file length)
   CHECK_ERROR(fstat(fd, &finfo) < 0);
   // Memory map the file
   CHECK_ERROR((fdata = mmap(0, finfo.st_size + 1, 
      PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0)) == NULL);

   // Setup scheduler args
   scheduler_args_t sched_args;
   memset(&sched_args, 0, sizeof(scheduler_args_t));
   sched_args.task_data = fdata; // Array to regress
   sched_args.map = linear_regression_map;
   sched_args.reduce = linear_regression_reduce; // Identity Reduce
   sched_args.splitter = NULL; // Array splitter;
   sched_args.key_cmp = intkeycmp;
   sched_args.unit_size = sizeof(POINT_T);
   sched_args.partition = linear_regression_partition; 
   sched_args.result = &final_vals;
   sched_args.data_size = finfo.st_size - (finfo.st_size % sched_args.unit_size);
   sched_args.L1_cache_size = atoi(GETENV("MR_L1CACHESIZE")); //1024 * 64;
   sched_args.num_map_threads = atoi(GETENV("MR_NUMTHREADS"));//8;
   sched_args.num_reduce_threads = atoi(GETENV("MR_NUMTHREADS"));//16;
   sched_args.num_merge_threads = atoi(GETENV("MR_NUMTHREADS")) / 2;//8;
   sched_args.num_procs = atoi(GETENV("MR_NUMPROCS"));//16;
   sched_args.key_match_factor = (float)atof(GETENV("MR_KEYMATCHFACTOR"));//2;

   printf("Linear Regression: Calling MapReduce Scheduler\n");

   gettimeofday(&starttime,0);

   CHECK_ERROR(map_reduce_scheduler(&sched_args) < 0);

   long long n;
   double a, b, xbar, ybar, r2;
   long long SX_ll = 0, SY_ll = 0, SXX_ll = 0, SYY_ll = 0, SXY_ll = 0;
   // ADD UP RESULTS
   for (i = 0; i < final_vals.length; i++)
   {
      keyval_t * curr = &final_vals.data[i];
      switch ((int)curr->key)
      {
      case KEY_SX:
          SX_ll = (*(long long*)curr->val);
          break;
      case KEY_SY:
          SY_ll = (*(long long*)curr->val);
          break;
      case KEY_SXX:
          SXX_ll = (*(long long*)curr->val);
          break;
      case KEY_SYY:
          SYY_ll = (*(long long*)curr->val);
          break;
      case KEY_SXY:
          SXY_ll = (*(long long*)curr->val);
          break;
      default:
          // INVALID KEY
          CHECK_ERROR(1);
      }
      free(curr->val);
   }

   double SX = (double)SX_ll;
   double SY = (double)SY_ll;
   double SXX= (double)SXX_ll;
   double SYY= (double)SYY_ll;
   double SXY= (double)SXY_ll;

   n = (long long) finfo.st_size / sizeof(POINT_T); 
   //b = (double)(n*SXY/SX - SY) / (n*SXX/SX - SX);
   b = (double)(n*SXY - SX*SY) / (n*SXX - SX*SX);
   a = (SY_ll - b*SX_ll) / n;
   xbar = (double)SX_ll / n;
   ybar = (double)SY_ll / n;
   r2 = (double)(n*SXY - SX*SY) * (n*SXY - SX*SY) / ((n*SXX - SX*SX)*(n*SYY - SY*SY));

   gettimeofday(&endtime,0);

   printf("Linear Regression: Completed %ld\n",(endtime.tv_sec - starttime.tv_sec));

   printf("Linear Regression Results:\n");
   printf("\ta    = %lf\n", a);
   printf("\tb    = %lf\n", b);
   printf("\txbar = %lf\n", xbar);
   printf("\tybar = %lf\n", ybar);
   printf("\tr2   = %lf\n", r2);
   printf("\tSX   = %lld\n", SX_ll);
   printf("\tSY   = %lld\n", SY_ll);
   printf("\tSXX  = %lld\n", SXX_ll);
   printf("\tSYY  = %lld\n", SYY_ll);
   printf("\tSXY  = %lld\n", SXY_ll);

   free(final_vals.data);

   CHECK_ERROR(munmap(fdata, finfo.st_size + 1) < 0);
   CHECK_ERROR(close(fd) < 0);

   return 0;
}


