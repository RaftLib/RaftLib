#include <thread>
#include <cmath>
#include <cstdint>
#include <cstddef>
#include <cstdio>
#ifdef __linux
#include <sys/sysinfo.h>
#endif
#include <errno.h>
#include <cassert>

#include "affinity.hpp"

#ifdef __linux

/** for get cpu **/
#if (__GLIBC_MINOR__ < 14) && (__GLIBC__ <= 2)

#ifndef _GNU_SOURCE
#define _GNU_SOURCE 1
#endif

#else

#ifndef _BSD_SOURCE
#define _BSD_SOURCE 1
#endif

#ifndef _SVID_SOURCE
#define _SVID_SOURCE 1
#endif

#endif /** end glibc check **/

#include <sched.h>
#endif /** end if linux **/


void 
affinity::set( const std::size_t desired_core )
{
#ifdef __linux
   /**
    * pin the current thread 
    */
   cpu_set_t   *cpuset( nullptr );
   auto cpu_allocate_size( -1 );
#if   (__GLIBC_MINOR__ > 9 ) && (__GLIBC__ == 2 )
   const auto  processors_to_allocate( 1 );
   cpuset = CPU_ALLOC( processors_to_allocate );
   assert( cpuset != nullptr );
   cpu_allocate_size = CPU_ALLOC_SIZE( processors_to_allocate );
   CPU_ZERO_S( cpu_allocate_size, cpuset );
#else
   cpu_allocate_size = sizeof( cpu_set_t );
   cpuset = (cpu_set_t*) malloc( cpu_allocate_size );
   assert( cpuset != nullptr );
   CPU_ZERO( cpuset );
#endif
   CPU_SET( desired_core,
            cpuset );
   errno = 0;
   if( sched_setaffinity( 0 /* calling thread */,
                         cpu_allocate_size,
                         cpuset ) != 0 )
   {
      perror( "Failed to set affinity for cycle counter!!" );
      exit( EXIT_FAILURE );
   }
   /** wait till we know we're on the right processor **/
   if( sched_yield() != 0 )
   {
      perror( "Failed to yield to wait for core change!\n" );
   }
#else /** not linux **/
#warning "No thread pinning for this platform, your results may vary!"
#endif
   return;
}
