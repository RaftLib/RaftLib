// (C) Copyright Princeton University 2009
// Written by Christian Bienia
// Implementation of pthreads-style barriers based on pthread mutexes
// Use to replace pthread barriers on machines that have pthreads but not pthread barriers

// Some comments on this barrier implementation and directions for possible improvements:
//
//   * This is a straightforward two-phase centralized barrier implementation
//   * It's main advantage is that it only requries the pthreads library (no atomic instructions),
//     making it very portable and easy to maintain if libpthread is already installed
//   * The only more advanced feature is the ability to spin (if enabled). This should yield
//     significant performance improvements if the number of threads is less or equal to the
//     number of processors
//   * There are many ways to improve this implementation, just check the scientific literature.
//     Some suggestions are:
//       - Dynamically adapt maximum amount of spinning based on observed runtime behavior
//       - Tree-based or "butterfly" barriers (John M. Mellor-Crumm, TOCS 1991)
//       - Adaptive combining tree barriers (Michael L. Scott et al, IJPP 1994)

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <errno.h>
#include <assert.h>

#include "parsec_barrier.hpp"

#ifdef USERAFTLIB
#include <raft>
#endif

//Internal debugging functions
#define NOT_IMPLEMENTED() Not_Implemented(__FUNCTION__,__FILE__,__LINE__);

static inline void Not_Implemented(const char* function,const char* file,unsigned int line) {
  fprintf(stderr, "ERROR: Something in function %s in file %s, line %u is not implemented.\n", function, file, line);
  exit(1);
}

#ifdef ENABLE_SPIN_BARRIER
//Counter type to use for spinning
typedef unsigned long long int spin_counter_t;

//Maximum amount of iterations to spin on flag before falling back to blocking
//NOTE: A value of 350 corresponds to about 1 us on modern computers
//Set value so we spin no more than 0.1 ms
static const spin_counter_t SPIN_COUNTER_MAX=350*100;
#endif //ENABLE_SPIN_BARRIER


//Barrier initialization & destruction
int parsec_barrier_init(parsec_barrier_t *barrier, const parsec_barrierattr_t *attr, unsigned count) {
  int rv;

#if defined(DEBUG) && defined(ENABLE_AUTOMATIC_DROPIN)
  //Print a notification once if automatic drop-in is enabled
  static int flag = 0;
  if(!flag) {
    printf("PARSEC barrier drop-in replacement enabled.\n");
    flag = 1;
  }
#endif //DEBUG

  //check assumptions used in header
  assert(PARSEC_BARRIER_SERIAL_THREAD != 0);

  //check arguments
  if(barrier==NULL) return EINVAL;
  if(count<=0) return EINVAL;
  //only private barriers (the default) are currently supported
  if(attr!=NULL && *attr==PARSEC_PROCESS_PRIVATE) NOT_IMPLEMENTED();

  barrier->max = count;
  barrier->n = 0;
  barrier->is_arrival_phase = 1;

  rv = pthread_mutex_init(&barrier->mutex, NULL);
  if(rv != 0) return rv;
  rv= pthread_cond_init(&barrier->cond, NULL);
  return rv;
}

int parsec_barrier_destroy(parsec_barrier_t *barrier) {
  int rv;

  assert(barrier!=NULL);

  rv = pthread_mutex_destroy(&barrier->mutex);
  if(rv != 0) return rv;
  rv= pthread_cond_destroy(&barrier->cond);
  if(rv != 0) return rv;

  //If the barrier is still in use then the pthread_*_destroy functions should
  //have returned an error, but we check anyway to catch any other unexpected errors.
  if(barrier->n != 0) return EBUSY;
  return 0;
}

//Barrier attribute initialization & destruction
int parsec_barrierattr_destroy(parsec_barrierattr_t *attr) {
  if(attr==NULL) return EINVAL;
  //simply do nothing
  return 0;
}

int parsec_barrierattr_init(parsec_barrierattr_t *attr) {
  if(attr==NULL) return EINVAL;
  //simply do nothing
  return 0;
}

//Barrier attribute modification
int parsec_barrierattr_getpshared(const parsec_barrierattr_t *attr, int *pshared) {
  if(attr==NULL || pshared==NULL) return EINVAL;
  *pshared = *attr;
  return 0;
}

int parsec_barrierattr_setpshared(parsec_barrierattr_t *attr, int pshared) {
  if(attr==NULL) return EINVAL;
  if(pshared!=PARSEC_PROCESS_SHARED && pshared!=PARSEC_PROCESS_PRIVATE) return EINVAL;
  //Currently we only support private barriers (the default)
  if(pshared!=PARSEC_PROCESS_PRIVATE) NOT_IMPLEMENTED();
  *attr = pshared;
  return 0;
}

//Barrier usage
int parsec_barrier_wait(parsec_barrier_t *barrier) {
  int master;
  int rv;

  if(barrier==NULL) return EINVAL;

  rv = pthread_mutex_lock(&barrier->mutex);
  if(rv != 0) return rv;
  //First we must wait to be sure that all threads from a previous barrier use had
  //the chance to leave (departure phase complete) before we can reuse the barrier
#ifndef ENABLE_SPIN_BARRIER
  //Standard method to block on a condition variable (with simple error propagation)
  while(!barrier->is_arrival_phase) {
    rv = pthread_cond_wait(&barrier->cond, &barrier->mutex);
    if(rv != 0) {
      pthread_mutex_unlock(&barrier->mutex);
      return rv;
    }
  }
#else
  //A (necessarily) unsynchronized polling loop followed by fall-back blocking
  if(!barrier->is_arrival_phase) {
    pthread_mutex_unlock(&barrier->mutex);
    volatile spin_counter_t i=0;
    while(!barrier->is_arrival_phase && i<SPIN_COUNTER_MAX) i++;
    while((rv=pthread_mutex_trylock(&barrier->mutex)) == EBUSY);
    if(rv != 0) return rv;

    //Fall back to normal waiting on condition variable if necessary
    while(!barrier->is_arrival_phase) {
      rv = pthread_cond_wait(&barrier->cond, &barrier->mutex);
      if(rv != 0) {
        pthread_mutex_unlock(&barrier->mutex);
        return rv;
      }
    }
  }
#endif //ENABLE_SPIN_BARRIER

  //We are guaranteed to be in an arrival phase, proceed with barrier synchronization
  master = (barrier->n == 0); //Make first thread at barrier the master
  barrier->n++;
  if(barrier->n >= barrier->max) {
    //This is the last thread to arrive, don't wait instead
    //start a new departure phase and wake up all other threads
    barrier->is_arrival_phase = 0;
    pthread_cond_broadcast(&barrier->cond);
  } else {
    //wait for last thread to arrive (which will end arrival phase)
#ifndef ENABLE_SPIN_BARRIER
    //Standard method to block on a condition variable
    while(barrier->is_arrival_phase) pthread_cond_wait(&barrier->cond, &barrier->mutex);
#else
    //we use again an unsynchronized polling loop followed by synchronized fall-back blocking
    if(barrier->is_arrival_phase) {
      pthread_mutex_unlock(&barrier->mutex);
      volatile spin_counter_t i=0;
      while(barrier->is_arrival_phase && i<SPIN_COUNTER_MAX) i++;
      while((rv=pthread_mutex_trylock(&barrier->mutex)) == EBUSY);
      if(rv != 0) return rv;

      //Fall back to normal waiting on condition variable if necessary
      while(barrier->is_arrival_phase) {
        rv = pthread_cond_wait(&barrier->cond, &barrier->mutex);
        if(rv != 0) {
          pthread_mutex_unlock(&barrier->mutex);
          return rv;
        }
      }
    }
#endif //ENABLE_SPIN_BARRIER
  }
  barrier->n--;
  //last thread to leave barrier starts a new arrival phase
  if(barrier->n == 0) {
    barrier->is_arrival_phase = 1;
    pthread_cond_broadcast(&barrier->cond);
  }
  pthread_mutex_unlock(&barrier->mutex);

  return (master ? PARSEC_BARRIER_SERIAL_THREAD : 0);
}



//Uncomment this macro to add a small program for debugging purposes
//#define ENABLE_BARRIER_CHECKER

#ifdef ENABLE_BARRIER_CHECKER

#include <sched.h>

typedef unsigned long long int test_counter_t;

//some global variables
const int PRIME_NUMBER=31; //An arbitrarily chosen prime to mix up access patterns a little
const int NTHREADS=4; //Number of threads to check barrier with
const int NBUGGERS=3; //Maximum number of `bugger' threads to use
const int WORK_UNIT_WORKER=2*131072; //Work unit for tester threads (must be power of two due to overflow)
const int WORK_UNIT_BUGGER=WORK_UNIT_WORKER/128; //Work unit for bugger threads (must be power of two due to overflow)
volatile int terminate_bugger_threads; //flag to signal termination to bugger threads

parsec_barrier_t barrier;

volatile test_counter_t result_worker[NTHREADS];
volatile test_counter_t result_bugger[NBUGGERS];


//A simple background thread, designed to disturb the timing behavior of the worker threads
void *bugger_thread(void *arg) {
  int tid = *(int *)arg;
  int i;

  assert(tid>=0 && tid<NBUGGERS);
  assert(result_bugger[tid]==0);

  //do something simple to occupy the processor and yield to cause unexpected delays
  while(!terminate_bugger_threads) {
    for(i=0; i<WORK_UNIT_BUGGER; i++) {
      result_bugger[tid]++;
    }
    sched_yield();
  }

  return NULL;
}

//A synthetic barrier stress test
void *stress_thread(void *arg) {
  int tid = *(int *)arg;
  int i;

  assert(tid>=0 && tid<NTHREADS);
  assert(result_worker[tid]==0);

  //Simple barrier stress test
  for(i=0; i<WORK_UNIT_WORKER; i++) {
    parsec_barrier_wait(&barrier);
    result_worker[tid]++;
  }

  return NULL;
}

//A test incrementing counters in parallel using barrier synchronization
void *counter_thread(void *arg) {
  int tid = *(int *)arg;
  int i;

  assert(tid>=0 && tid<NTHREADS);
  assert(result_worker[tid]==0);

  //Increment a set of counters in parallel, pseudo-randomly pick which counter to increment
  //All counter values are always the same after each step, unless there's a race condition
  for(i=0; i<WORK_UNIT_WORKER; i++) {
    unsigned int idx;
    test_counter_t temp;

    //Step 1: Pseudo-randomly pick an array element
    parsec_barrier_wait(&barrier);
    idx=(PRIME_NUMBER*(result_worker[tid]+tid)) % NTHREADS;
    temp=result_worker[idx];
    //Step 2: Store new result in thread's own array element
    parsec_barrier_wait(&barrier);
    temp++;
    result_worker[tid]=temp;
  }

  return NULL;
}

int main(int argc, char **argv) {
  pthread_t workers[NTHREADS];
  pthread_t buggers[NBUGGERS];
  int worker_tids[NTHREADS];
  int bugger_tids[NBUGGERS];
  int i,j;

  printf("Starting barrier check program. Barrier options:\n");
#ifdef ENABLE_SPIN_BARRIER
  printf("  - Spin barriers: ENABLED\n");
#else
  printf("  - Spin barriers: DISABLED\n");
#endif

  //the barrier to test
  parsec_barrier_init(&barrier, NULL, NTHREADS);

  //Initialize thread IDs
  for(i=0; i<=NBUGGERS; i++) bugger_tids[i]=i;
  for(i=0; i<=NTHREADS; i++) worker_tids[i]=i;

  printf("Phase 1: Barrier stress test\n");
  for(i=0; i<=NBUGGERS; i++) {
    printf("  Starting test with %i bugger thread(s)\n", i);
    terminate_bugger_threads=0;
    for(j=0; j<i; j++) {
      result_bugger[j]=0;
      pthread_create(&buggers[j], NULL, bugger_thread, &bugger_tids[j]);
    }

    //start check
    for(j=0; j<NTHREADS; j++) {
      result_worker[j]=0;
      pthread_create(&workers[j], NULL, stress_thread, &worker_tids[j]);
    }
    for(j=0; j<NTHREADS; j++) {
      pthread_join(workers[j], NULL);
      //check result
      if(result_worker[j] % WORK_UNIT_WORKER != 0) {
        printf("ERROR: Incorrect result for worker thread %i.\n", j);
      }
    }

    //check is over, terminate bugger threads
    terminate_bugger_threads=1;
    for(j=0; j<i; j++) {
      pthread_join(buggers[j], NULL);
      //check result
      if(result_bugger[j] % WORK_UNIT_BUGGER != 0) {
        printf("ERROR: Incorrect result for bugger thread %i.\n", j);
      }
    }
  }

  printf("Phase 2: Parallel counter test\n");
  for(i=0; i<=NBUGGERS; i++) {
    printf("  Starting test with %i bugger thread(s)\n", i);
    terminate_bugger_threads=0;
    for(j=0; j<i; j++) {
      result_bugger[j]=0;
      pthread_create(&buggers[j], NULL, bugger_thread, &bugger_tids[j]);
    }

    //start check
    for(j=0; j<NTHREADS; j++) {
      result_worker[j]=0;
      pthread_create(&workers[j], NULL, counter_thread, &worker_tids[j]);
    }
    for(j=0; j<NTHREADS; j++) {
      pthread_join(workers[j], NULL);
      //check result
      if(result_worker[j] % WORK_UNIT_WORKER != 0) {
        printf("ERROR: Incorrect result for worker thread %i.\n", j);
      }
    }

    //check is over, terminate bugger threads
    terminate_bugger_threads=1;
    for(j=0; j<i; j++) {
      pthread_join(buggers[j], NULL);
      //check result
      if(result_bugger[j] % WORK_UNIT_BUGGER != 0) {
        printf("ERROR: Incorrect result for bugger thread %i.\n", j);
      }
    }
  }

  parsec_barrier_destroy(&barrier);
  return 0;
}
#endif //ENABLE_BARRIER_CHECKER
