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
#include <time.h>

#include "MapReduceScheduler.h"
#include "stddefines.h"

typedef struct {
   int row_num;
   int *matrix_A;
   int *matrix_B;
   int matrix_len;
   int unit_size;
   int row_block_len;
   int* output;
} mm_data_t;

/* Structure to store the coordinates
and location for each value in the matrix */
typedef struct {
	int x_loc;
	int y_loc;
	int value;
} mm_key_t;

int count = 0;

/** myintcmp()
 *  Comparison Function to compare 2 locations in the matrix
 */
int myintcmp(const void *v1, const void *v2)
{
   mm_key_t* key1 = (mm_key_t*)v1;
   mm_key_t* key2 = (mm_key_t*)v2;

   if(key1->x_loc < key2->x_loc) return -1;
   else if(key1->x_loc > key2->x_loc) return 1;
   else
   {
	   if(key1->y_loc < key2->y_loc) return -1;
	   else if(key1->y_loc > key2->y_loc) return 1;
	   else return 0;
   }
}

/** matrixmul_splitter()
 *  Assign a set of rows of the output matrix to each map task
 */
int matrixmult_splitter(void *data_in, int req_units, map_args_t *out)
{
	/* Make a copy of the mm_data structure */
   mm_data_t * data = (mm_data_t *)data_in; 
   mm_data_t * data_out = (mm_data_t *)malloc(sizeof(mm_data_t));
   memcpy((char*)data_out,(char*)data,sizeof(mm_data_t));

   /* Check whether the various terms exist */
   assert(data_in);
   assert(out);
   assert(req_units >= 0);
   
   assert(data->matrix_len >= 0);
   assert(data->unit_size >= 0);
   assert(data->row_block_len >= 0);

   assert(data->matrix_A);
   assert(data->matrix_B);

   assert(data->row_num <= data->matrix_len);
   
   dprintf("Required units is %d\n",req_units);

	 /* Reached the end of the matrix */
   if(data->row_num >= data->matrix_len)
   {
	   fflush(stdout);
	   free(data_out);
	   return 0;
   }

		/* Compute available rows */
   int available_rows = data->matrix_len - data->row_num;
   out->length = (req_units < available_rows)? req_units:available_rows;
   out->data = data_out;

   data->row_num += out->length;
   dprintf("Allocated rows is %d\n",out->length);

   return 1;
}

/** matrixmul_map()
 * Multiplies the allocated regions of matrix to compute partial sums 
 */
void matrixmult_map(map_args_t *args)
{
   int row_count = 0;
   int i,j, x_loc, y_loc,value;
   int * a_ptr,* b_ptr;   

   assert(args);
   
   mm_data_t* data = (mm_data_t*)(args->data);
   assert(data);

   dprintf("In Map task %d %d\n",data->row_num, args->length);

   dprintf("%d Start Loop \n",data->row_num);

   while(row_count < args->length)
   {
	   a_ptr = data->matrix_A + (data->row_num + row_count)*data->matrix_len;

	   for(i=0; i < data->matrix_len ; i++)
	   {
		   b_ptr = data->matrix_B + i;
		   value = 0;

		   for(j=0;j<data->matrix_len ; j++)
		   {
			   value += ( a_ptr[j] * (*b_ptr));
			   b_ptr+= data->matrix_len;
		   }
		   x_loc = (data->row_num + row_count);
		   y_loc = i;
		   data->output[x_loc*data->matrix_len + i] = value;
		   fflush(stdout);
	   }
	   dprintf("%d Loop\n",data->row_num);
	   
      row_count++;	
   }

   dprintf("Finished Map task %d\n",data->row_num);

   fflush(stdout);
   free(args->data);
}

/** matrixmul_reduce()
 * Adds up the partial sums for each point in the matrix
 */
void matrixmult_reduce(void *key_in, void **vals_in, int vals_len)
{
   int i;
   printf("\n\n matrixmult_reduce\n\n");

   for (i = 0; i < vals_len; i++)
   {
      emit(key_in, vals_in[i]);
   }
}

int main(int argc, char *argv[]) {

   final_data_t mm_vals;
   int i,j, create_files;
   int fd_A, fd_B, fd_out,file_size;
   char * fdata_A, *fdata_B;
   int matrix_len, row_block_len;
   struct stat finfo_A, finfo_B;
   char * fname_A, *fname_B,*fname_out;
   int *matrix_A_ptr, *matrix_B_ptr;

   struct timeval starttime,endtime;
   
   srand( (unsigned)time( NULL ) );

   // Make sure a filename is specified
   if (argv[1] == NULL)
   {
      dprintf("USAGE: %s [side of matrix] [size of Row block]\n", argv[0]);
      exit(1);
   }

   fname_A = "matrix_file_A.txt";
   fname_B = "matrix_file_B.txt";
   fname_out = "matrix_file_out.txt";
   CHECK_ERROR ( (matrix_len = atoi(argv[1])) < 0);
   file_size = ((matrix_len*matrix_len))*sizeof(int);

   fprintf(stderr, "***** file size is %d\n", file_size);

   if(argv[2] == NULL)
	   row_block_len = 1;
   else
	   CHECK_ERROR ( (row_block_len = atoi(argv[2])) < 0);

   if(argv[3] != NULL)
	   create_files = 1;
   else
	   create_files = 0;

   printf("MatrixMult: Side of the matrix is %d\n", matrix_len);
   printf("MatrixMult: Row Block Len is %d\n", row_block_len);
   printf("MatrixMult: Running...\n");

   CHECK_ERROR((fd_out = open(fname_out,O_CREAT | O_RDWR,S_IRWXU)) < 0);

	 /* If the matrix files do not exist, create them */
   if(create_files)
   {
	   dprintf("Creating files\n");

	   int value = 0;
	   CHECK_ERROR((fd_A = open(fname_A,O_CREAT | O_RDWR,S_IRWXU)) < 0);
	   CHECK_ERROR((fd_B = open(fname_B,O_CREAT | O_RDWR,S_IRWXU)) < 0);
	   
	   for(i=0;i<matrix_len;i++)
	   {
		   for(j=0;j<matrix_len;j++)
		   {
			   value = (rand())%11;
			   write(fd_A,&value,sizeof(int));
			   dprintf("%d  ",value);
		   }
		   dprintf("\n");
	   }
	   dprintf("\n");

	   for(i=0;i<matrix_len;i++)
	   {
		   for(j=0;j<matrix_len;j++)
		   {
			   value = (rand())%11;
			   write(fd_B,&value,sizeof(int));
			   dprintf("%d  ",value);
		   }
		   dprintf("\n");
	   }

	   CHECK_ERROR(close(fd_A) < 0);
	   CHECK_ERROR(close(fd_B) < 0);
   }

   // Read in the file
   CHECK_ERROR((fd_A = open(fname_A,O_RDONLY)) < 0);
   // Get the file info (for file length)
   CHECK_ERROR(fstat(fd_A, &finfo_A) < 0);

   // Memory map the file
   CHECK_ERROR((fdata_A= mmap(0, file_size + 1,
      PROT_READ | PROT_WRITE, MAP_PRIVATE, fd_A, 0)) == NULL);

   // Read in the file
   CHECK_ERROR((fd_B = open(fname_B,O_RDONLY)) < 0);
   // Get the file info (for file length)
   CHECK_ERROR(fstat(fd_B, &finfo_B) < 0);
   // Memory map the file
   CHECK_ERROR((fdata_B= mmap(0, file_size + 1,
      PROT_READ | PROT_WRITE, MAP_PRIVATE, fd_B, 0)) == NULL);

   // Setup splitter args
   mm_data_t mm_data;
   mm_data.unit_size = row_block_len*matrix_len*sizeof(int); 
   mm_data.matrix_len = matrix_len;
   mm_data.row_block_len = row_block_len;
   mm_data.matrix_A = NULL;
   mm_data.matrix_B = NULL;
   mm_data.row_num = 0;

   mm_data.output = (int*)malloc(matrix_len*matrix_len*sizeof(int));
   
   mm_data.matrix_A = matrix_A_ptr = ((int *)fdata_A);
   mm_data.matrix_B = matrix_B_ptr = ((int *)fdata_B);

   // Setup scheduler args
   scheduler_args_t sched_args;
   sched_args.task_data = &mm_data;
   sched_args.map = matrixmult_map;
   sched_args.reduce = matrixmult_reduce;
   sched_args.splitter = matrixmult_splitter;
   sched_args.key_cmp = myintcmp;
   sched_args.unit_size = mm_data.unit_size;
   sched_args.partition = NULL; // use default
   sched_args.result = &mm_vals;
   sched_args.data_size = file_size;
   sched_args.L1_cache_size = atoi(GETENV("MR_L1CACHESIZE")); //1024 * 64;
   sched_args.num_map_threads = atoi(GETENV("MR_NUMTHREADS"));//8;
   sched_args.num_reduce_threads = atoi(GETENV("MR_NUMTHREADS"));//16;
   sched_args.num_merge_threads = atoi(GETENV("MR_NUMTHREADS")) / 2;//8;
   sched_args.num_procs = atoi(GETENV("MR_NUMPROCS"));//16;
   sched_args.key_match_factor = (float)atof(GETENV("MR_KEYMATCHFACTOR"));//2;

	fprintf(stderr, "***** data size is %d\n", sched_args.data_size);
   printf("MatrixMult: Calling MapReduce Scheduler Matrix Multiplication\n");

	gettimeofday(&starttime,0);

   CHECK_ERROR(map_reduce_scheduler(&sched_args) < 0);

   gettimeofday(&endtime,0);

   printf("MatrixMult: Multiply Completed time = %ld\n", (endtime.tv_sec - starttime.tv_sec));

   dprintf("\n");
   //dprintf("The length of the final output is %d\n",mm_vals.length );
   for(i=0;i<matrix_len*matrix_len;i++)
   {
	  write(fd_out,&(mm_data.output[i]),sizeof(int));
   }
   dprintf("\n");

   dprintf("MatrixMult: MapReduce Completed\n");

   free(mm_vals.data);
   free(mm_data.output);

   CHECK_ERROR(munmap(fdata_A, file_size + 1) < 0);
   CHECK_ERROR(close(fd_A) < 0);

   CHECK_ERROR(munmap(fdata_B, file_size + 1) < 0);
   CHECK_ERROR(close(fd_B) < 0);

   CHECK_ERROR(close(fd_out) < 0);

   return 0;
}
