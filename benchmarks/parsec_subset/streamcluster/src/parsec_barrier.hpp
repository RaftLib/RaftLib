// (C) Copyright Princeton University 2009
// Written by Christian Bienia
// Implementation of pthreads-style barriers based on pthread mutexes
// Use to replace pthread barriers on machines that have pthreads but not pthread barriers

#ifndef __PARSEC_BARRIER_H_
#define __PARSEC_BARRIER_H_ 1

#include <pthread.h>
#include <errno.h>



//This code functions as a drop-in replacement for pthread barriers.
//Read the documentation for the corresponding pthread_barrier* calls
//to understand how this code is used (not all pthread features are implemented)

//If defined then macros will be used to redefine the relevant pthread_barrier*
//symbols to the equivalent parsec_barrier* symbols. This will make the host code
//use the parsec_barrier* replacement calls without the need to touch the source
//code (other than including this header file), but it makes it harder to understand
//what is going on
#define ENABLE_AUTOMATIC_DROPIN

//Whether to allow the use of spinning. If enabled then the barrier implementation
//will busy-wait on a flag first. After a pre-determined amount of time has passed
//without any success it will fall back to waiting on a condition variable. Spinning
//will result in unsynchronized memory accesses to the flag.
#define ENABLE_SPIN_BARRIER



#ifdef ENABLE_AUTOMATIC_DROPIN
//Redefine all calls to pthread_barrier* functions in host code by using macros
#undef pthread_barrier_t
#undef pthread_barrierattr_t
#undef pthread_barrier_init
#undef pthread_barrier_destroy
#undef pthread_barrierattr_init
#undef pthread_barrierattr_destroy
#undef pthread_barrierattr_getpshared
#undef pthread_barrierattr_setpshared
#undef pthread_barrier_wait

#define pthread_barrier_t parsec_barrier_t
#define pthread_barrierattr_t parsec_barrier_t
#define pthread_barrier_init(b, a, c) parsec_barrier_init(b, a, c)
#define pthread_barrier_destroy(b) parsec_barrier_destroy(b)
#define pthread_barrierattr_init(a) parsec_barrierattr_init(a)
#define pthread_barrierattr_destroy(a) parsec_barrierattr_destroy(a)
#define pthread_barrierattr_getpshared(a) parsec_barrierattr_getpshared(a)
#define pthread_barrierattr_setpshared(a) parsec_barrierattr_setpshared(a)
#define pthread_barrier_wait(b) parsec_barrier_wait(b)

//Do the same with constants
#undef PTHREAD_BARRIER_SERIAL_THREAD
#undef PTHREAD_PROCESS_SHARED
#undef PTHREAD_PROCESS_PRIVATE

#define PTHREAD_BARRIER_SERIAL_THREAD PARSEC_BARRIER_SERIAL_THREAD
#define PTHREAD_PROCESS_SHARED PARSEC_PROCESS_SHARED
#define PTHREAD_PROCESS_PRIVATE PARSEC_PROCESS_PRIVATE
#endif //ENABLE_AUTOMATIC_DROPIN



typedef int parsec_barrierattr_t;

//Type definitions
typedef struct {
  pthread_mutex_t mutex;
  pthread_cond_t cond;
  unsigned max;
  volatile unsigned n;
  volatile int is_arrival_phase;
} parsec_barrier_t;

//pthread_barrier_wait can never return EINTR, use that as default value
const int PARSEC_BARRIER_SERIAL_THREAD = EINTR;
const parsec_barrierattr_t PARSEC_PROCESS_SHARED = 0;
const parsec_barrierattr_t PARSEC_PROCESS_PRIVATE = 1;



//Barrier initialization & destruction
int parsec_barrier_init(parsec_barrier_t *barrier, const parsec_barrierattr_t *attr, unsigned count);
int parsec_barrier_destroy(parsec_barrier_t *barrier);

//Barrier attribute initialization & destruction
int parsec_barrierattr_destroy(parsec_barrierattr_t *attr);
int parsec_barrierattr_init(parsec_barrierattr_t *attr);

//Barrier attribute modification
int parsec_barrierattr_getpshared(const parsec_barrierattr_t *attr, int *pshared);
int parsec_barrierattr_setpshared(parsec_barrierattr_t *attr, int pshared);

//Barrier usage
int parsec_barrier_wait(parsec_barrier_t *barrier);

#endif //__PARSEC_BARRIER_H_
