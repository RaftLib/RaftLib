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

#include "MapReduceScheduler.h"

#include <assert.h>
#include <pthread.h>

#include <sys/unistd.h>
#include <unistd.h>
#include <stdlib.h>

#include <stdio.h>
#include <errno.h>
#include <string.h>

#include <strings.h>

#ifdef _LINUX_
#include <sched.h>
#endif

#ifdef _SOLARIS_
#include <sys/procset.h>
#include <sys/processor.h>
#include <sys/types.h>
#endif

#if !defined(_LINUX_) && !defined(_SOLARIS_)
#error Operating system not supported. Currently _LINUX_ and _SOLARIS_ (Solaris) are supported.
#endif

#define DEFAULT_CACHE_SIZE (64 * 1024)
#define DEFAULT_KEYVAL_ARR_LEN 10
#define DEFAULT_VALS_ARR_LEN 10

//#ifdef MAPRED_TIMING
//#include <time.h>
//#endif

/* Debug printf */
#define dprintf(...) //printf(__VA_ARGS__)

/* Wrapper to check for errors */
#define CHECK_ERROR(a)                                       \
   if ((a))                                                  \
   {                                                         \
      perror("Error at line\n\t" #a "\nSystem Msg");         \
      exit(1);                                               \
   }

#define MIN(X,Y) ((X) < (Y) ? (X) : (Y))

typedef struct 
{
   int len;
   int alloc_len;
   keyval_t *arr;
} keyval_arr_t;

typedef struct
{
   int len;
   int alloc_len;
   void * key;
   void ** vals;
} keyvals_t;

typedef struct 
{
   int len;
   int alloc_len;
   keyvals_t *arr;
} keyvals_arr_t;

typedef struct 
{
   pthread_t tid;
   int cpuid;
   int isTransientError;
   int curr_task;
} thread_info_t;

// Global state, only one of these 
// thus this program is not thread-safe
struct
{
   int map_tasks;             // max number of map tasks
   int reduce_tasks;          // max number of reduce tasks
   int chunk_size;            // number of units of data for each map task
   int num_procs;             // number of processors to run on
   int num_map_threads;       // number of threads to create for map tasks
   int num_reduce_threads;    // number of threads to create for reduce tasks
   int num_merge_threads;     // number of threads to create for merge tasks
   float key_match_factor;    // number of values likely to be matched to the same key
   reduce_t reduce;           // reduce function to run
   partition_t partition;     // partition function to use
   splitter_t splitter;       // splitter function to use
   int splitter_pos;          // Used by array_splitter() to track position
   int isOneQueuePerTask;         // used to indicate if each map/reduce task should
                              // share an output queue with the other tasks
                              // running on the same processor or not
   int isOneQueuePerReduceTask;
   scheduler_args_t * args;   // args passed in by the user
   thread_info_t * tinfo;     // array of thread ids and CPU ids currently running
   
   keyvals_arr_t ** intermediate_vals; // array to send to reduce task
   int intermediate_task_alloc_len;
   keyval_arr_t * final_vals;         // array to send to merge task
   keyval_arr_t * merge_vals;         // array to send to user

   int transient_map_faults;
   int transient_reduce_faults;
   int transient_merge_faults;

   int permanent_map_faults;
   int permanent_reduce_faults;
   int permanent_merge_faults;

   pthread_mutex_t fault_lock; // Lock for splitter_func()
} g_state;

typedef enum {      // constants for func_type in the thread_wrapper_arg_t struct
   MAP,
   REDUCE,
   MERGE,
   // add more if needed
} TASK_TYPE_T;

typedef struct
{
   int cpu_id;                   // CPU this thread is running on
   TASK_TYPE_T func_type;        // Indicates which function to run tasks
   pthread_mutex_t *splitter_lock; // Lock for splitter_func()
   
   int *pos;
   int merge_len;
   keyval_arr_t * merge_input;
}  thread_wrapper_arg_t;

static inline void scheduler_init(scheduler_args_t *args);
static inline void schedule_tasks(thread_wrapper_arg_t *);
static inline int getNumProcs(void);
static inline int isCpuAvailable(unsigned long, int);
static inline void setCpuAvailable(unsigned long *, int);
static inline void * MALLOC(size_t);
static inline void * CALLOC(size_t, size_t);
static inline void * REALLOC(void *, size_t);
static inline int min(int, int);
static inline int max(int, int);

static inline int getCurrThreadIndex(TASK_TYPE_T task_type);
static inline int getNumTaskThreads(TASK_TYPE_T task_type);
static inline void insert_keyval(keyval_arr_t *, void *, void *);
static inline void insert_keyval_merged(keyvals_arr_t *, void *, void *);

static int array_splitter(void *, int, map_args_t *);
static void identity_reduce(void *, void **, int);
static inline void merge_results(keyval_arr_t*, int);

static void *map_worker(void *);
static void *reduce_worker(void *);
static void *merge_worker(void *);

int map_reduce_scheduler(scheduler_args_t * args)
{

   thread_wrapper_arg_t th_arg; // Data passed to task scheduler

   assert(args); assert(args->map); 
   assert(args->key_cmp); assert(args->unit_size > 0); assert(args->result);

   scheduler_init(args);


   memset(&th_arg, 0, sizeof(thread_wrapper_arg_t));
   th_arg.func_type = MAP;

   // Run map tasks and get intermediate values. 
   schedule_tasks(&th_arg);  


   dprintf("In scheduler, all map tasks are done, now scheduling reduce tasks\n");

   memset(&th_arg, 0, sizeof(thread_wrapper_arg_t));
   th_arg.func_type = REDUCE;

   // Run reduce tasks and get final values. 
   schedule_tasks(&th_arg);
   

   // Cleanup intermediate results
   free(g_state.intermediate_vals);

   dprintf("In scheduler, all reduce tasks are done, now scheduling merge tasks\n");
   memset(&th_arg, 0, sizeof(thread_wrapper_arg_t));
   th_arg.func_type = MERGE;
   
   if (g_state.isOneQueuePerReduceTask) {
      th_arg.merge_len = g_state.reduce_tasks;
   }
   else {
      th_arg.merge_len = g_state.num_reduce_threads;
   }
   th_arg.merge_input = g_state.final_vals;

   if (th_arg.merge_len <= 1) {
      g_state.args->result->data = g_state.final_vals->arr;
      g_state.args->result->length = g_state.final_vals->len;

      free(g_state.final_vals);
   }
   else {
      while (th_arg.merge_len > 1)
      {
         // This is the worst case length, 
         // depending on the value of num_merge_threads
         g_state.merge_vals = (keyval_arr_t*) 
            MALLOC((th_arg.merge_len / 2) * sizeof(keyval_arr_t));
   
         // Run merge tasks and get merge values. 
         schedule_tasks(&th_arg);
         
         free(th_arg.merge_input);
         th_arg.merge_len = th_arg.merge_len / 2;
         if (th_arg.merge_len >  g_state.num_merge_threads)
            th_arg.merge_len = g_state.num_merge_threads;
         th_arg.merge_input = g_state.merge_vals;
      }
      
      g_state.args->result->data = g_state.merge_vals[0].arr;
      g_state.args->result->length = g_state.merge_vals[0].len;
      free(g_state.merge_vals);
   }

   return 0;
}

// Setup global state
static void scheduler_init(scheduler_args_t *args) 
{
   int i;

   memset(&g_state, 0, sizeof(g_state));
   g_state.args = args;

   // Determine the number of processors to use   
   int num_procs = getNumProcs();
   if (args->num_procs > 0)
   {
      // Can't have more processors than there are physically
      CHECK_ERROR (args->num_procs > num_procs);
      num_procs = args->num_procs;
   }
   g_state.num_procs = num_procs;

   g_state.isOneQueuePerTask = args->use_one_queue_per_task;
   g_state.isOneQueuePerReduceTask = 1;
   
   // Determine the number of threads to schedule for each task
   g_state.num_map_threads = (args->num_map_threads > 0) ? 
         args->num_map_threads : num_procs;

   g_state.num_reduce_threads = (args->num_reduce_threads > 0) ? 
         args->num_reduce_threads : num_procs;

   g_state.num_merge_threads = (args->num_merge_threads > 0) ? 
         args->num_merge_threads : g_state.num_reduce_threads/2;
   
   if (g_state.isOneQueuePerReduceTask) {
      g_state.num_merge_threads = min(num_procs, 
                     max((g_state.reduce_tasks/2), g_state.num_merge_threads));
   }
   
   g_state.key_match_factor = (args->key_match_factor > 0) ? 
         args->key_match_factor : 2;

   g_state.transient_map_faults = 0;
   g_state.transient_reduce_faults = 0;
   g_state.transient_merge_faults = 0;

   g_state.permanent_map_faults = 0;
   g_state.permanent_reduce_faults = 0;
   g_state.permanent_merge_faults = 0;

   

   CHECK_ERROR(pthread_mutex_init(&g_state.fault_lock, NULL) != 0); 

   g_state.map_tasks = 0;
   if (args->L1_cache_size > 0)
   {   
      g_state.chunk_size = args->L1_cache_size / args->unit_size;
      g_state.reduce_tasks = (int) ( (g_state.key_match_factor * args->data_size) / (args->L1_cache_size) );
   }
   else 
   {      
      g_state.chunk_size = DEFAULT_CACHE_SIZE / args->unit_size;
      g_state.reduce_tasks = (int) ( (g_state.key_match_factor * args->data_size) / (DEFAULT_CACHE_SIZE) );
   }

   if (g_state.reduce_tasks <= 0) g_state.reduce_tasks = 1;
   if (g_state.chunk_size <= 0) g_state.chunk_size = 1;

   g_state.reduce_tasks = 256;


   if (g_state.isOneQueuePerTask) 
      g_state.intermediate_task_alloc_len = args->data_size/g_state.chunk_size + 1;
   else
      g_state.intermediate_task_alloc_len = g_state.num_map_threads;

   g_state.intermediate_vals = (keyvals_arr_t **)MALLOC(
         g_state.reduce_tasks*sizeof(keyvals_arr_t*));

   for (i = 0; i < g_state.reduce_tasks; i++)
   {
      g_state.intermediate_vals[i] = (keyvals_arr_t *)CALLOC(
         g_state.intermediate_task_alloc_len, sizeof(keyvals_arr_t));
   }

   if (g_state.isOneQueuePerReduceTask) 
   {
      g_state.final_vals = 
         (keyval_arr_t *) CALLOC(g_state.reduce_tasks, sizeof(keyval_arr_t));
   }
   else 
   {
      g_state.final_vals = 
         (keyval_arr_t *) CALLOC(g_state.num_reduce_threads, sizeof(keyval_arr_t));
   }
   
   g_state.merge_vals = NULL; // Will equal the total number of emitted values

   // Determine which functions to call
   g_state.partition = (args->partition != NULL) ? 
         args->partition : default_partition;

   g_state.reduce = (args->reduce != NULL) ? 
         args->reduce : identity_reduce;

   g_state.splitter = (args->splitter != NULL) ? 
         args->splitter : array_splitter;

   // we just set g_state.map_tasks to 0 coz we really have no idea how many map
   // tasks there are going to be, but it doesn't matter, coz the map_worker can
   // just loop till there is no more data left.   
   
  
}

/** schedule_tasks()
 *  thread_func - function pointer to process splitter data
 *  splitter_func - splitter function pointer
 *  splitter_init - splitter_init function pointer
 *  runs map tasks in a new thread on each the available processors.
 *  returns pointer intermediate value array 
 */
static inline void schedule_tasks(thread_wrapper_arg_t *th_arg)
{
   assert(th_arg);

   pthread_attr_t attr;   // parameter for pthread creation
   thread_wrapper_arg_t * curr_th_arg; // arg for thread_wrapper()
   
   int thread_cnt;        // counter of number threads assigned assigned
   int curr_proc;
   int curr_thread;

   int num_threads = getNumTaskThreads(th_arg->func_type);
   int threads_per_proc = num_threads / g_state.num_procs; 
   int threads_mod_procs = num_threads % g_state.num_procs;

   int pos = 0; // position of next result in the array
   pthread_mutex_t splitter_lock; // lock for splitter function

   g_state.tinfo = (thread_info_t *)CALLOC(num_threads, sizeof(thread_info_t));
   CHECK_ERROR(pthread_mutex_init(&splitter_lock, NULL) != 0);   
   
   dprintf("Number of available processors = %d\n", g_state.num_procs);
   dprintf("Number of Threads to schedule = %d per(%d) mod(%d)\n", 
      num_threads, threads_per_proc, threads_mod_procs);

   th_arg->pos = &pos;
   th_arg->splitter_lock = &splitter_lock;
   
   // thread must be scheduled systemwide
   pthread_attr_init(&attr);
   pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);

#ifdef _LINUX_
   unsigned long cpu_set; // bit array of available processors
   // Create a thread for each availble processor to handle the split data
   CHECK_ERROR(sched_getaffinity(0, sizeof(cpu_set), &cpu_set) == -1);
   for (thread_cnt = curr_proc = 0; 
        curr_proc < sizeof(cpu_set) && thread_cnt < num_threads; 
        curr_proc++)
   {
      if (isCpuAvailable(cpu_set, curr_proc))
      {
#endif
#ifdef _SOLARIS_
   int max_procs = sysconf(_SC_NPROCESSORS_ONLN);
   for (thread_cnt = curr_proc = 0; thread_cnt < num_threads; curr_proc++)
   {
      if (P_ONLINE == p_online(curr_proc, P_STATUS))
      {
#endif
         
         for (curr_thread = !(threads_mod_procs-- > 0); 
              curr_thread <= threads_per_proc && thread_cnt < num_threads; 
              curr_thread++, thread_cnt++)
         {
            // Setup data to be passed to each thread
            curr_th_arg = (thread_wrapper_arg_t*)MALLOC(sizeof(thread_wrapper_arg_t));
            memcpy(curr_th_arg, th_arg, sizeof(thread_wrapper_arg_t));
            curr_th_arg->cpu_id = curr_proc;

            g_state.tinfo[thread_cnt].cpuid = curr_proc;

            //fprintf(stderr, "Starting thread %d on cpu %d\n", thread_cnt, curr_th_arg->cpu_id);
            switch (th_arg->func_type)
            {
            case MAP:
               CHECK_ERROR(pthread_create(&g_state.tinfo[thread_cnt].tid, &attr, 
                                                map_worker, curr_th_arg) != 0);
               break;
            case REDUCE:
               CHECK_ERROR(pthread_create(&g_state.tinfo[thread_cnt].tid, &attr, 
                                                reduce_worker, curr_th_arg) != 0);
               break;
            case MERGE:
               CHECK_ERROR(pthread_create(&g_state.tinfo[thread_cnt].tid, &attr, 
                                                merge_worker, curr_th_arg) != 0);
               break;
            default:
               assert(0);
               break;
            }
         }
      }
      
      /*** ADDED BY RAM TO ASSIGN EACH PTHREAD TO HARDWARE THREADS ON DIFFERENT
      PROCESSORS ON THE ULTRASPARC T1 ****/
      if (getenv("MR_AFARA") != NULL)
      {
         //fprintf(stderr, "Using sparse threads\n");
         curr_proc += 3;
         if (curr_proc >= max_procs-1) {
            curr_proc++;
            curr_proc = curr_proc % max_procs; 
         }
      }
   }

   
   dprintf("Status: All %d threads have been created\n", num_threads);
   
   // barrier, wait for all threads to finish           
   for (thread_cnt = 0; thread_cnt < num_threads; thread_cnt++)
   {
      int ret_val;
      CHECK_ERROR(pthread_join(g_state.tinfo[thread_cnt].tid, (void **)(void *)&ret_val) != 0);
      
      // The thread returned and error. Restart the thread.
      //if (ret_val != 0)
      //{
      //}
   }
   
   pthread_attr_destroy(&attr);
   free(g_state.tinfo);
   dprintf("Status: All tasks have completed\n"); 
   
   return;
}

/** map_worker()
* args - pointer to thread_wrapper_arg_t
* returns 0 on success
* This runs thread_func() until there is no more data from the splitter().
* The pointer to results are stored in return_values array.
*/
static void *map_worker(void *args) 
{
   thread_wrapper_arg_t *th_arg = (thread_wrapper_arg_t *)args;
   int thread_index = getCurrThreadIndex(MAP);
   map_args_t thread_func_arg;
   int num_assigned = 0;
   int ret; // return value of splitter func. 0 = no more data to provide
   int isOneQueuePerTask = g_state.isOneQueuePerTask;

   assert(th_arg);
   
#ifdef _LINUX_
   // Bind thread to run on cpu_id
   unsigned long cpu_set = 0;
   setCpuAvailable(&cpu_set, th_arg->cpu_id);
   CHECK_ERROR(sched_setaffinity(0, sizeof(cpu_set), &cpu_set) != 0);
#endif

#ifdef _SOLARIS_
   dprintf("Binding thread to processor %d\n", th_arg->cpu_id);
   CHECK_ERROR(processor_bind(P_LWPID, P_MYID, th_arg->cpu_id, NULL)!= 0);
   /*if (processor_bind(P_LWPID, P_MYID, th_arg->cpu_id, NULL)!= 0) {
      switch(errno)
      {
         case EFAULT: dprintf("EFAULT\n");
                        break;
         case EINVAL: dprintf("EINVAL\n");
                        break;
         case EPERM:  dprintf("EPERM\n");
                        break;
         case ESRCH:  dprintf("ESRCH\n");
                        break;
         default: dprintf("Errno is %d\n",errno);
         
      }
   }*/
#endif

   while (1)
   {
      pthread_mutex_lock(th_arg->splitter_lock);
            
      ret = g_state.splitter(g_state.args->task_data, g_state.chunk_size, &thread_func_arg);
      if (ret != 0) 
      {
         int alloc_len = g_state.intermediate_task_alloc_len;
         g_state.tinfo[thread_index].curr_task = g_state.map_tasks++;
         num_assigned++;

         if (isOneQueuePerTask && g_state.map_tasks > alloc_len)
         {
            dprintf("MAP TASK QUEUE REALLOC\n");
            int i;

            g_state.intermediate_task_alloc_len *= 2;

            for (i = 0; i < g_state.reduce_tasks; i++)
            {
               g_state.intermediate_vals[i] = (keyvals_arr_t *)REALLOC(
                  g_state.intermediate_vals[i], 
                  g_state.intermediate_task_alloc_len*sizeof(keyvals_arr_t));
               memset(&g_state.intermediate_vals[i][alloc_len], 0, 
                  alloc_len*sizeof(keyvals_arr_t));
            }
         }
      }
      
      pthread_mutex_unlock(th_arg->splitter_lock);

      // Stop if there is no more data
      if (ret == 0) break;
      
      dprintf("Task %d: cpu_id -> %d - Started\n", num_assigned, th_arg->cpu_id);

      g_state.args->map(&thread_func_arg);

      dprintf("Task %d: cpu_id -> %d - Done\n", num_assigned, th_arg->cpu_id);
   }

   dprintf("Status: Total of %d tasks were assigned to cpu_id %d\n", 
      num_assigned, th_arg->cpu_id);

   free(args);
   
   return (void *)0;
}


static void *reduce_worker(void *args)
{
   thread_wrapper_arg_t *th_arg = (thread_wrapper_arg_t *)args;   
   int thread_index = getCurrThreadIndex(REDUCE);
   int isOneQueuePerTask = g_state.isOneQueuePerTask;
   
   assert(th_arg);
   
	#ifdef _LINUX_
	   // Bind thread to run on cpu_id
	   unsigned long cpu_set = 0;
	   setCpuAvailable(&cpu_set, th_arg->cpu_id);
	   CHECK_ERROR(sched_setaffinity(0, sizeof(cpu_set), &cpu_set) != 0);
	#endif

	#ifdef _SOLARIS_
	   CHECK_ERROR(processor_bind(P_LWPID, P_MYID, th_arg->cpu_id, NULL)!= 0);
	   /*if (processor_bind(P_LWPID, P_MYID, th_arg->cpu_id, NULL)!= 0) {
	      switch(errno)
         {
            case EFAULT: dprintf("EFAULT\n");
                           break;
            case EINVAL: dprintf("EINVAL\n");
                           break;
            case EPERM:  dprintf("EPERM\n");
                           break;
            case ESRCH:  dprintf("ESRCH\n");
                           break;
            default: dprintf("Errno is %d\n",errno);
            
         }   
	   }*/
	#endif

	int curr_thread, done;
   int curr_reduce_task = 0;
   int ret;
   int num_map_threads;
   if (isOneQueuePerTask)
      num_map_threads = g_state.map_tasks;
   else
      num_map_threads = g_state.num_map_threads;

   int startsize = DEFAULT_VALS_ARR_LEN;
   keyvals_arr_t* thread_array;
   int vals_len, max_len, next_min_pos;
   keyvals_t *curr_key_val, *min_key_val, *next_min;

   int * thread_position = (int *)MALLOC(num_map_threads * sizeof(int));
   void** vals = MALLOC(sizeof(char*)*startsize);

   while (1)
   {
      // Get the next reduce task
	   pthread_mutex_lock(th_arg->splitter_lock);
   	
      ret = (*th_arg->pos >= g_state.reduce_tasks);
      if (!ret)
      {
	      g_state.tinfo[thread_index].curr_task = curr_reduce_task = 
	                                                      (*th_arg->pos)++;
      }
      pthread_mutex_unlock(th_arg->splitter_lock);

      // No more reduce tasks
      if(ret) break;
   	
	   bzero((char *)thread_position, num_map_threads*sizeof(int));

	   vals_len = 0;
		max_len = startsize;
		
      min_key_val = NULL;
		next_min =  NULL;
      done = 0;

      while (!done)
      {
         for (curr_thread = 0; curr_thread < num_map_threads; curr_thread++)
         {
		      /* Find the next array to search */
		      thread_array = 
               &g_state.intermediate_vals[curr_reduce_task][curr_thread];

		      /* Check if the current processor array has been completely searched */
            if (thread_position[curr_thread] >= thread_array->len) continue;
         
		      /* Get the next key in the processor array */
		      curr_key_val = &thread_array->arr[thread_position[curr_thread]];

		      /* If the key matches the minimum value. Then add the value to the 
               list of values for that key */
            if (min_key_val != NULL && 
                !g_state.args->key_cmp(curr_key_val->key, min_key_val->key))
            {
               if (g_state.reduce == identity_reduce)
               {
                  int j;
                  for (j = 0; j < curr_key_val->len; j++)
                  {
				         emit_inline(min_key_val->key, curr_key_val->vals[j]);
                  }
               }
               else
               {
                  if (vals_len + curr_key_val->len >= max_len)
                  {
                     while (vals_len + curr_key_val->len >= max_len)
				            max_len *= 2;
   				      
                     vals = REALLOC(vals, sizeof(char*)*(max_len));
                  }
				      memcpy(&vals[vals_len], curr_key_val->vals, 
                        curr_key_val->len*sizeof(char*));
				      vals_len += curr_key_val->len;
               }

               thread_position[curr_thread--]++;
            }
		      /* Find the location of the next min */	
            else if (next_min == NULL || 
                     g_state.args->key_cmp(curr_key_val->key, next_min->key) < 0)
            {
               next_min = curr_key_val;
               next_min_pos = curr_thread;
            }
         }

	      if(min_key_val != NULL)
	      {
            if (g_state.reduce != identity_reduce)
            {
		         g_state.reduce(min_key_val->key, vals, vals_len);
               
            }

            vals_len = 0;
	         min_key_val = NULL;
         }

         if (next_min != NULL)
         {
            min_key_val = next_min;
            next_min = NULL;
         }
         
         // See if there are any elements left
         for(curr_thread = 0; curr_thread < num_map_threads && 
             thread_position[curr_thread] >= 
             g_state.intermediate_vals[curr_reduce_task][curr_thread].len; 
             curr_thread++);
	      done = (curr_thread == num_map_threads);
      }
      
      for (curr_thread = 0; curr_thread < num_map_threads; curr_thread++)
      {
         keyvals_arr_t * arr = &g_state.intermediate_vals[curr_reduce_task][curr_thread];
         int j;
         for(j = 0; j < arr->len; j++)
         {
            free(arr->arr[j].vals);
         }
         free(arr->arr);
      }
      free(g_state.intermediate_vals[curr_reduce_task]);
   }

   free(thread_position);
   free(vals);
   free(args);

   return (void *)0;
}

/** merge_worker()
* args - pointer to thread_wrapper_arg_t
* returns 0 on success
*/
static void *merge_worker(void *args) {
   
   thread_wrapper_arg_t *th_arg = (thread_wrapper_arg_t *)args;
   int curr_thread = getCurrThreadIndex(MERGE);

   assert(th_arg);
   
#ifdef _LINUX_
   // Bind thread to run on cpu_id
   unsigned long cpu_set = 0;
   setCpuAvailable(&cpu_set, th_arg->cpu_id);
   CHECK_ERROR(sched_setaffinity(0, sizeof(cpu_set), &cpu_set) != 0);
#endif

#ifdef _SOLARIS_
   CHECK_ERROR(processor_bind(P_LWPID, P_MYID, th_arg->cpu_id, NULL)!= 0);
   /*if (processor_bind(P_LWPID, P_MYID, th_arg->cpu_id, NULL)!= 0) {
      switch(errno)
      {
         case EFAULT: dprintf("EFAULT\n");
                        break;
         case EINVAL: dprintf("EINVAL\n");
                        break;
         case EPERM:  dprintf("EPERM\n");
                        break;
         case ESRCH:  dprintf("ESRCH\n");
                        break;
         default: dprintf("Errno is %d\n",errno);
         
      }
      
   }*/
#endif

   // Assumes num_merge_threads is modified before each call
   int length = th_arg->merge_len / g_state.num_merge_threads;
   int modlen = th_arg->merge_len % g_state.num_merge_threads;
   
   // Let's make some progress here
   if (length <= 1)
   {
      length = 2;
      modlen = th_arg->merge_len % 2;
   }

   int pos = curr_thread*length + ((curr_thread < modlen) ? curr_thread : modlen);

   if (pos < th_arg->merge_len)
   {
      keyval_arr_t *vals = &th_arg->merge_input[pos];

      dprintf("Thread %d: cpu_id -> %d - Started\n", 
         curr_thread, th_arg->cpu_id);

      merge_results(vals, length + (curr_thread < modlen));

      dprintf("Thread %d: cpu_id -> %d - Done\n", 
         curr_thread, th_arg->cpu_id);
   }
   free(args);

   return (void *)0;
}

/** emit_intermediate()
 *  inserts the key, val pair into the intermediate array
 */
inline void emit_intermediate_inline(void *key, void *val, int key_size)
{
   int curr_thread = getCurrThreadIndex(MAP);
   int isOneQueuePerTask = g_state.isOneQueuePerTask;
   int curr_task;

   if (isOneQueuePerTask)
      curr_task = g_state.tinfo[curr_thread].curr_task;
   else
      curr_task = curr_thread;
   
/*#ifdef MAPRED_TIMING
   struct timespec start_time, part_end_time, ins_end_time;
   clock_gettime(CLOCK_REALTIME, &start_time);
#endif      // MAPRED_TIMING*/

   
   int reduce_pos = g_state.partition(g_state.reduce_tasks, key, key_size);
   //if (reduce_pos >= g_state.reduce_tasks)
   //printf("WARNING: REDUCE BUCKET %d:%d\n", reduce_pos, g_state.reduce_tasks);
   reduce_pos %= g_state.reduce_tasks;
   
   
   /* Insert sorted in global queue at pos curr_proc */
   
/*#ifdef MAPRED_TIMING
   clock_gettime(CLOCK_REALTIME, &part_end_time);
#endif      // MAPRED_TIMING   */
   
   keyvals_arr_t * arr = 
      &g_state.intermediate_vals[reduce_pos][curr_task];

   insert_keyval_merged(arr, key, val);
   
/*#ifdef MAPRED_TIMING
   clock_gettime(CLOCK_REALTIME, &ins_end_time);
   GET_TIME(start_time, part_end_time, times[curr_thread].part_time);
   GET_TIME(part_end_time, ins_end_time, times[curr_thread].ins_time);
   times[curr_thread].cnt++;
#endif      // MAPRED_TIMING   */
}

void emit_intermediate(void *key, void *val, int key_size)
{
   emit_intermediate_inline(key, val, key_size);
}
/** emit()
 *  inserts the key, val pair into the final output array
 */
inline void emit_inline(void *key, void *val)
{
   int curr_red_queue;
   
   if (g_state.isOneQueuePerReduceTask) {
      int thread_index = getCurrThreadIndex(REDUCE);
      curr_red_queue = g_state.tinfo[thread_index].curr_task;
   }
   else {
      curr_red_queue = getCurrThreadIndex(REDUCE);
   }
   /* Insert sorted in global queue at pos curr_proc */
   
   keyval_arr_t * arr = &g_state.final_vals[curr_red_queue];
   insert_keyval(arr, key, val);
}

void emit(void *key, void *val)
{
   emit_inline(key, val);
}

static inline void insert_keyval_merged(keyvals_arr_t *arr, void *key, void *val)
{
   int thread_index = getCurrThreadIndex(MAP);
   int high = arr->len, low = -1, next;
   int cmp = 1;

   assert(arr->len <= arr->alloc_len);
   if (arr->len > 0)
      cmp = g_state.args->key_cmp(arr->arr[arr->len - 1].key, key);
   
   if (cmp > 0)
   {
      // Binary search the array to find the key
      while (high - low > 1)
      {
          next = (high + low) / 2;           
          if (g_state.args->key_cmp(arr->arr[next].key, key) > 0)
              high = next;
          else
              low = next;
      }
      
      if (low < 0) low = 0;
      if (arr->len > 0 && (cmp = g_state.args->key_cmp(arr->arr[low].key, key)) < 0) low++;
   }
   else if (cmp < 0)
      low = arr->len;
   else 
      low = arr->len-1;
      
   // We are simulating a transient error. This means the value is already in the array.
   // Do not insert it twice.
   if (g_state.tinfo[thread_index].isTransientError)
      return;
   
   if (arr->len == 0 || cmp)
   {
      // if array is full, double and copy over
      if (arr->len == arr->alloc_len)
      {
         if (arr->alloc_len == 0)
         {
            arr->alloc_len = DEFAULT_KEYVAL_ARR_LEN;
            arr->arr = (keyvals_t*)MALLOC(arr->alloc_len * sizeof(keyvals_t));
         }
         else
         {
            arr->alloc_len *= 2;
            arr->arr = (keyvals_t*)REALLOC(arr->arr, arr->alloc_len * sizeof(keyvals_t));
         }
      }
      
      // Insert into array
      memmove(&arr->arr[low+1], &arr->arr[low], (arr->len - low) * sizeof(keyvals_t));

      arr->arr[low].key = key;
      arr->arr[low].alloc_len = 0;
      arr->arr[low].len = 0;
      arr->arr[low].vals = NULL;
      arr->len++;
   }

   // Merge Value
   if (arr->arr[low].len == arr->arr[low].alloc_len)
   {
      if (arr->arr[low].alloc_len == 0)
      {
         arr->arr[low].alloc_len = DEFAULT_KEYVAL_ARR_LEN;
         arr->arr[low].vals = MALLOC(arr->arr[low].alloc_len * sizeof(int*));
      }
      else
      {
         arr->arr[low].alloc_len *= 2;
         arr->arr[low].vals = REALLOC(arr->arr[low].vals, 
            arr->arr[low].alloc_len * sizeof(int*));
      }
   }

   arr->arr[low].vals[arr->arr[low].len++] = val;
}

static inline void insert_keyval(keyval_arr_t *arr, void *key, void *val)
{
   int thread_index = getCurrThreadIndex(REDUCE);
   int high = arr->len, low = -1, next;
   int cmp = 1;

   assert(arr->len <= arr->alloc_len);
   
   if (arr->len > 0)
      cmp = g_state.args->key_cmp(arr->arr[arr->len - 1].key, key);
   
   if (cmp > 0)
   {
      // Binary search the array to find the key
      while (high - low > 1)
      {
          next = (high + low) / 2;           
          if (g_state.args->key_cmp(arr->arr[next].key, key) > 0)
              high = next;
          else
              low = next;
      }
      
      if (low < 0) low = 0;
      if (arr->len > 0 && g_state.args->key_cmp(arr->arr[low].key, key) < 0) low++;
   }
   else
      low = arr->len;
   
   // We are simulating a transient error. This means the value is already in the array.
   // Do not insert it twice.
   if (g_state.tinfo[thread_index].isTransientError)
      return;

   // if array is full, double and copy over
   if (arr->len == arr->alloc_len)
   {
      if (arr->alloc_len == 0)
      {
         arr->alloc_len = DEFAULT_KEYVAL_ARR_LEN;
         arr->arr = (keyval_t*)MALLOC(arr->alloc_len * sizeof(keyval_t));
      }
      else
      {
         arr->alloc_len *= 2;
         arr->arr = (keyval_t*)REALLOC(arr->arr, arr->alloc_len * sizeof(keyval_t));
      }
   }
   
   // Insert into array
   memmove(&arr->arr[low+1], &arr->arr[low], (arr->len - low) * sizeof(keyval_t));

   arr->arr[low].key = key;
   arr->arr[low].val = val;
   arr->len++;
}

static inline void merge_results(keyval_arr_t *vals, int length) 
{
   keyval_t *min_keyval;
   int cmp_ret;
   int data_idx;
   int total_num_keys = 0;
   int min_idx;
   int i;
   int curr_thread = getCurrThreadIndex(MERGE);

   // stores the current index for each list of reduce outputs 
   int * curr_index = (int *)CALLOC(length, sizeof(int));
   
   for (i = 0; i < length; i++) 
   {
      total_num_keys += vals[i].len;
   }

   g_state.merge_vals[curr_thread].len = total_num_keys;
   g_state.merge_vals[curr_thread].alloc_len = total_num_keys;
   g_state.merge_vals[curr_thread].arr = (keyval_t *)
      MALLOC(sizeof(keyval_t) * total_num_keys);
   
   for (data_idx = 0; data_idx < total_num_keys; data_idx++) 
   {
      for (i = 0; i < length &&
           curr_index[i] >= vals[i].len; i++); 

      if (i == length) break;
      
      min_idx = i;
      min_keyval = &vals[i].arr[curr_index[i]];
      
      for (i++; i < length; i++) 
      {  
         if (curr_index[i] < vals[i].len)
         {
            cmp_ret = g_state.args->key_cmp(min_keyval->key, 
               vals[i].arr[curr_index[i]].key);         

            if (cmp_ret > 0) 
            {
               min_idx = i;
               min_keyval = &vals[i].arr[curr_index[i]];
            }
         }
      }

      memcpy(&g_state.merge_vals[curr_thread].arr[data_idx], min_keyval, sizeof(keyval_t));
      curr_index[min_idx]++;
   }
      
   free(curr_index);
   for (i = 0; i < length; i++) 
   {
      free(vals[i].arr);
   }
}

static inline int getNumTaskThreads(TASK_TYPE_T task_type)
{
   int num_threads;
   
   switch (task_type)
   {
   case MAP:
      num_threads = g_state.num_map_threads;
      break;
   case REDUCE:
      num_threads = g_state.num_reduce_threads;
      break;
   case MERGE:
      num_threads = g_state.num_merge_threads;
      break;
   default:
      assert(0);
      num_threads = g_state.num_map_threads;
      break;
   }

   return num_threads;
}

/** getCurrThreadIndex()
 *  Returns the processor the current thread is running on
 */
static inline int getCurrThreadIndex(TASK_TYPE_T task_type)
{
   int i;
   int num_threads = getNumTaskThreads(task_type);
   pthread_t mytid = pthread_self();

   for (i = 0; i < num_threads && g_state.tinfo[i].tid != mytid; i++);

   assert(i != num_threads);

   return i;
}

/** getNumProc()
 *  returns the number of available processors
 */
static inline int getNumProcs(void)
{
   char* num_proc_str = NULL;
   int num_procs = 0;

#ifdef _LINUX_
   unsigned long cpus;
   int i;
   // Returns number of processors available to process (based on affinity mask)
   CHECK_ERROR(sched_getaffinity(0, sizeof(cpus), &cpus) == -1);
   for (i = 0; i < sizeof(cpus); i++)
   {
      num_procs += isCpuAvailable(cpus, i);
   }
#endif

#ifdef _SOLARIS_
   CHECK_ERROR((num_procs = sysconf(_SC_NPROCESSORS_ONLN)) <= 0);
#endif

   // Check if the user specified a different number of processors 
   if ((num_proc_str = getenv("MAPRED_NPROCESSORS")) != NULL)
   {
      int temp = atoi(num_proc_str);
      CHECK_ERROR (temp < 1 || temp > num_procs);
      num_procs = temp;
   }

   return num_procs;
}

/** isCpuAvailable()
 *  cpu_set - bit array of processors available
 *  cpu - index of cpu to check in cpu_set
 *  return 1 if available, 0 if not.
 */
static inline int isCpuAvailable(unsigned long cpu_set, int cpu)
{
   assert(cpu < sizeof(cpu_set) * 8 && cpu >= 0);
   return ((1<<cpu) & cpu_set) != 0;
}

/** setCpuAvailable()
 *  cpu_set - bit array of processors available
 *  cpu - index of cpu to check in cpu_set
 *  this makes cpu index available in cpu_set.
 */
static inline void setCpuAvailable(unsigned long * cpu_set, int cpu)
{
   assert(cpu < sizeof(*cpu_set) * 8 && cpu >= 0);
   *cpu_set |= (1<<cpu);
}

static inline void * MALLOC(size_t size)
{
   void * temp = malloc(size);
   assert(temp);
   return temp;
}

static inline void * REALLOC(void *ptr, size_t size)
{
   void * temp = realloc(ptr, size);
   assert(temp);
   return temp;
}

static inline void * CALLOC(size_t num, size_t size)
{
   void * temp = calloc(num, size);
   assert(temp);
   return temp;
}

/** array_splitter()
 *
 */
static int array_splitter(void *data_in, int req_units, map_args_t *out)
{
   assert(out);
   int unit_size = g_state.args->unit_size;
   int data_units = g_state.args->data_size / unit_size;

   // End of data reached, return FALSE
   if (g_state.splitter_pos >= data_units) return 0;
   
   // Set the start of the next data
   out->data = ((char *)g_state.args->task_data) + g_state.splitter_pos*unit_size;
   
   // Determine the nominal length
   if (g_state.splitter_pos + req_units > data_units)
      out->length = data_units - g_state.splitter_pos;
   else
      out->length = req_units;

   g_state.splitter_pos += out->length;

   // Return true since the out data is valid
   return 1;
}

static void identity_reduce(void *key, void **vals, int vals_len)
{
   int i;
   for (i = 0; i < vals_len; i++)
   {
      emit_inline(key, vals[i]);
   }
}

int default_partition(int reduce_tasks, void* key, int key_size)
{
   unsigned long hash = 5381;
   char *str = (char *)key;
   int i;

   for (i = 0; i < key_size; i++)
   {
      hash = ((hash << 5) + hash) + ((int)str[i]); /* hash * 33 + c */
   }

   return hash % reduce_tasks;
}


static inline int min(int a, int b) {
   return (a < b ? a : b);   
}

static inline int max(int a, int b) {
   return (a > b ? a : b);
}
   
