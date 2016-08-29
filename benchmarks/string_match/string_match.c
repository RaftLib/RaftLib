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

#define DEFAULT_UNIT_SIZE 5
#define SALT_SIZE 2
#define MAX_REC_LEN 1024
#define OFFSET 5

typedef struct {
  int keys_file_len;
  int encrypted_file_len;
  long bytes_comp;
  char * keys_file;
  char * encrypt_file;
} str_data_t;

typedef struct {
  char * keys_file;
  char * encrypt_file;
} str_map_data_t;

 char *key1 = "Helloworld";
 char *key2 = "howareyou";
 char *key3 = "ferrari";
 char *key4 = "whotheman";

 char *key1_final;
 char *key2_final;
 char *key3_final;
 char *key4_final;

/** getnextline()
 *  Function to get the next word
 */
int getnextline(char* output, int max_len, char* file)
{
	int i=0;
	while(i<max_len-1)
	{
		if( file[i] == '\0')
		{
			if(i==0)
				return -1;
			else
				return i;
		}
		if( file[i] == '\r')
			return (i+2);

		if( file[i] == '\n' )
			return (i+1);

		output[i] = file[i];
		i++;
	}
	file+=i;
	return i;
}

/** mystrcmp()
 *  Default comparison function
 */
int mystrcmp(const void *v1, const void *v2)
{
  return 1;
}

/** compute_hashes()
 *  Simple Cipher to generate a hash of the word 
 */
void compute_hashes(char* word, char* final_word)
{
	int i;

	for(i=0;i<strlen(word);i++)
		final_word[i] = word[i]+OFFSET;
}

/** string_match_splitter()
 *  Splitter Function to assign portions of the file to each map task
 */
int string_match_splitter(void *data_in, int req_units, map_args_t *out)
{
	/* Make a copy of the mm_data structure */
   str_data_t * data = (str_data_t *)data_in; 
   str_map_data_t *map_data = (str_map_data_t *)malloc(sizeof(str_map_data_t));

   map_data->encrypt_file = data->encrypt_file;
   map_data->keys_file = data->keys_file + data->bytes_comp;

   /* Check whether the various terms exist */
   assert(data_in);
   assert(out);
   assert(req_units >= 0);

   assert(data->bytes_comp <= data->keys_file_len);

   if(data->bytes_comp >= data->keys_file_len)
   {
	   return 0;
   }

	 /* Assign the required number of bytes */
   int req_bytes = req_units*DEFAULT_UNIT_SIZE;
   int available_bytes = data->keys_file_len - data->bytes_comp;
   if(available_bytes < 0)
	   available_bytes = 0;

   out->length = (req_bytes < available_bytes)? req_bytes:available_bytes;
   out->data = map_data;

   char* final_ptr = map_data->keys_file + out->length;
   int counter = data->bytes_comp + out->length;

		/* make sure we end at a word */
   while(counter <= data->keys_file_len && *final_ptr != '\n'
		 && *final_ptr != '\r' && *final_ptr != '\0')
   {
	   counter++;
	   final_ptr++;
   }
   if(*final_ptr == '\r')
	   counter+=2;
   else if(*final_ptr == '\n')
	   counter++;

   out->length = counter - data->bytes_comp;
   data->bytes_comp = counter;

   return 1;
}

/** string_match_map()
 *  Map Function that checks the hash of each word to the given hashes
 */
void string_match_map(map_args_t *args)
{
   assert(args);
   
   str_map_data_t* data_in = (str_map_data_t*)(args->data);

	int key_len, total_len = 0;
	char * key_file = data_in->keys_file;
	char * cur_word = malloc(MAX_REC_LEN);
	char * cur_word_final = malloc(MAX_REC_LEN);
	bzero(cur_word, MAX_REC_LEN);
	bzero(cur_word_final, MAX_REC_LEN);

	while( (total_len <= args->length) && ((key_len = getnextline(cur_word, MAX_REC_LEN, key_file)) >= 0))
    {
		compute_hashes(cur_word, cur_word_final);

	   if(!strcmp(key1_final, cur_word_final))
		   dprintf(stderr,"FOUND: WORD IS %s\n", cur_word);

	   if(!strcmp(key2_final, cur_word_final))
		   dprintf(stderr,"FOUND: WORD IS %s\n", cur_word);

	   if(!strcmp(key3_final, cur_word_final))
		   dprintf(stderr,"FOUND: WORD IS %s\n", cur_word);

	   if(!strcmp(key4_final, cur_word_final))
		   dprintf(stderr,"FOUND: WORD IS %s\n", cur_word);

		key_file = key_file + key_len;
		bzero(cur_word,MAX_REC_LEN);
		bzero(cur_word_final, MAX_REC_LEN);
		total_len+=key_len;
   }
   free(cur_word);
   free(args->data);
   //printf("End thread %d\n", args->length);
}

/** string_match_reduce()
 *  Null function
 */
void string_match_reduce(void *key_in, void **vals_in, int vals_len)
{
}

int main(int argc, char *argv[]) {
   final_data_t str_vals;
   int fd_encrypt, fd_keys;
   char * fdata_encrypt, *fdata_keys;
   struct stat finfo_encrypt, finfo_keys;
   char * fname_encrypt, *fname_keys;

	 /* Option to provide the encrypted words in a file as opposed to source code */
   //fname_encrypt = "encrypt.txt";

   if (argv[1] == NULL)
   {
      printf("USAGE: %s <keys filename>\n", argv[0]);
      exit(1);
   }
   fname_keys = argv[1];

   struct timeval starttime,endtime;
   srand( (unsigned)time( NULL ) );

   printf("String Match: Running...\n");

   /*// Read in the file
   CHECK_ERROR((fd_encrypt = open(fname_encrypt,O_RDONLY)) < 0);
   // Get the file info (for file length)
   CHECK_ERROR(fstat(fd_encrypt, &finfo_encrypt) < 0);
   // Memory map the file
   CHECK_ERROR((fdata_encrypt= mmap(0, finfo_encrypt.st_size + 1,
      PROT_READ | PROT_WRITE, MAP_PRIVATE, fd_encrypt, 0)) == NULL);*/

   // Read in the file
   CHECK_ERROR((fd_keys = open(fname_keys,O_RDONLY)) < 0);
   // Get the file info (for file length)
   CHECK_ERROR(fstat(fd_keys, &finfo_keys) < 0);
   // Memory map the file
   CHECK_ERROR((fdata_keys= mmap(0, finfo_keys.st_size + 1,
      PROT_READ | PROT_WRITE, MAP_PRIVATE, fd_keys, 0)) == NULL);

   // Setup splitter args

	//dprintf("Encrypted Size is %ld\n",finfo_encrypt.st_size);
	dprintf("Keys Size is %ld\n",finfo_keys.st_size);

   str_data_t str_data;

   str_data.keys_file_len = finfo_keys.st_size;
   str_data.encrypted_file_len = 0;
   str_data.bytes_comp = 0;
   str_data.keys_file  = ((char *)fdata_keys);
   str_data.encrypt_file  = NULL;
   //str_data.encrypted_file_len = finfo_encrypt.st_size;
   //str_data.encrypt_file  = ((char *)fdata_encrypt);

   // Setup scheduler args
   scheduler_args_t sched_args;
   sched_args.task_data = &str_data;
   sched_args.map = string_match_map;
   sched_args.reduce = string_match_reduce;
   sched_args.splitter = string_match_splitter;
   sched_args.key_cmp = mystrcmp;
   sched_args.unit_size = DEFAULT_UNIT_SIZE;
   sched_args.partition = NULL; // use default
   sched_args.result = &str_vals;
   sched_args.data_size = finfo_keys.st_size;
   sched_args.L1_cache_size = atoi(GETENV("MR_L1CACHESIZE")); //1024 * 64;
   sched_args.num_map_threads = atoi(GETENV("MR_NUMTHREADS"));//8;
   sched_args.num_reduce_threads = atoi(GETENV("MR_NUMTHREADS"));//16;
   sched_args.num_merge_threads = atoi(GETENV("MR_NUMTHREADS")) / 2;//8;
   sched_args.num_procs = atoi(GETENV("MR_NUMPROCS"));//16;
   sched_args.key_match_factor = (float)atof(GETENV("MR_KEYMATCHFACTOR"));//2;

   printf("String Match: Calling String Match\n");

	key1_final = malloc(strlen(key1));
	key2_final = malloc(strlen(key2));
	key3_final = malloc(strlen(key3));
	key4_final = malloc(strlen(key4));

	compute_hashes(key1, key1_final);
	compute_hashes(key2, key2_final);
	compute_hashes(key3, key3_final);
	compute_hashes(key4, key4_final);

   gettimeofday(&starttime,0);

   CHECK_ERROR(map_reduce_scheduler(&sched_args) < 0);

   gettimeofday(&endtime,0);

   free(key1_final);
   free(key2_final);
   free(key3_final);
   free(key4_final);

   printf("String Match: Completed %ld\n",(endtime.tv_sec - starttime.tv_sec));

   /*CHECK_ERROR(munmap(fdata_encrypt, finfo_encrypt.st_size + 1) < 0);
   CHECK_ERROR(close(fd_encrypt) < 0);*/

   CHECK_ERROR(munmap(fdata_keys, finfo_keys.st_size + 1) < 0);
   CHECK_ERROR(close(fd_keys) < 0);

   return 0;
}
