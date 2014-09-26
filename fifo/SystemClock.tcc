/**
 * SystemClock.hpp - 
 * @author: Jonathan Beard
 * @version: Sun Apr 20 15:41:37 2014
 * 
 * Copyright 2014 Jonathan Beard
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef _SYSTEMCLOCK_HPP_
#define _SYSTEMCLOCK_HPP_  1
#include <cstdint>
#include <cstdlib>
#include <functional>
#include <cassert>
#include <cstdint>
#include <cstring>
#include <unistd.h>
#include <pthread.h>
#include "Clock.hpp"

#ifdef __APPLE__
#include <mach/mach.h>
#include <mach/mach_time.h>
#endif

#ifdef __linux
#include <time.h>
#ifndef _GNU_SOURCE
#define _GNU_SOURCE 1
#endif
#include <sched.h>
#endif


/**
 * TODO:
 * 1) Add new constructor for allocating in SHM
 * 2) Add accessor for getting SHM Key to open
 * 3) Add accessor to open SHM for new processes 
 *    to use the counter.
 * 4) Fix the latency issue (time for system calls and assembly code 
 */

enum ClockType  { Dummy, Cycle, System };

template < ClockType T > class SystemClock : public Clock {
public:
   SystemClock( int core  = 0) : updater( 0 )
   {
      thread_data.core = core;
      init();
   }

   virtual ~SystemClock()
   {
      thread_data.done = true;
      pthread_join( updater, nullptr );
   }

   
   virtual sclock_t getTime()
   {
      return( thread_data.clock->read() );
   }



private:

   void init()
   {
      errno = 0;
      if( pthread_create( &updater, 
                          nullptr, 
                          updateClock, 
                          (void*) &thread_data ) != 0 )
      {
         perror( "Failed to create timer thread, exiting." );
         exit( EXIT_FAILURE );
      }
      while( ! thread_data.setup )
      {
#ifdef __x86_64         
         __asm__ volatile ("pause" : : :);
#endif      
      }
   }

   class Clock {
   public:
      Clock() : a( (sclock_t) 0 ),
                b( (sclock_t) 0 )
      {}
      
      inline void increment( const sclock_t inc = (sclock_t) 1 )
      {
         a += inc;
         b += inc;
      }

      inline sclock_t read()
      {
         struct{
            sclock_t a;
            sclock_t b;
         } copy;
         do{
            copy.a = a;
            copy.b = b;
         }while( copy.a != copy.b );
         return( copy.b );
      }

   private:
      volatile sclock_t a;
      volatile sclock_t b;
   };

   struct ThreadData{
      ThreadData() : clock( nullptr ),
                     done(  false ),
                     setup( false ),
                     core( 0 )
      {
         clock = new Clock();
      }

      ~ThreadData()
      {
         delete( clock );
         clock = nullptr;
         done = true;
      }

      Clock         *clock;
      volatile bool done;
      volatile bool setup;
      int           core;
   } thread_data ;

   /**
    * updateClock - function called by thread to update global clock,
    */
   static void* updateClock( void *data )
   {
      ThreadData *d( reinterpret_cast< ThreadData* >( data ) );
      Clock         *clock( d->clock );
      volatile bool &done(   d->done );
      std::function< void () > function;
      switch( T )
      {
         case( Dummy ):
         {
            function = [&](){ clock->increment(); };
         }
         break;
         case( Cycle ):
         {
#ifdef   __linux
            FILE  *fp = NULL;
            errno = 0;
            fp = fopen("/proc/cpuinfo", "r");
            if(fp == NULL)
            {
               perror( "Failed to read proc/cpuinfo!!\n" );
               exit( EXIT_FAILURE );
            }
            const size_t buff_size = 20;
            char *key = (char*) alloca(sizeof(char) * buff_size);
            char *value = (char*) alloca(sizeof(char) * buff_size);
            assert( key != NULL );
            assert( value != NULL );
            std::memset( key, '\0', buff_size );
            std::memset( value, '\0', buff_size );
            uint64_t frequency( 0 );
            int count = EOF;
            while( ( count = fscanf(fp,"%[^:]:%[^\n]\n", key, value) ) != EOF )
            {
               if( count == 2 ){
                  /* TODO, not the best way to get CPU Frequency */
                  if( strncmp( key, "cpu MHz", 7 ) == 0 )
                  {
                     frequency = ( uint64_t ) (atof( value ) * 1e6f );
                     goto END;
                  }
               }
            }
         END:   
            fclose( fp );
                     
            /**
             * pin the current thread 
             */
            cpu_set_t   *cpuset( nullptr );
            size_t cpu_allocate_size( -1 );
#if   (__GLIBC_MINOR__ > 9 ) && (__GLIBC__ == 2 )
            const int8_t   processors_to_allocate( 1 );
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
            /** TODO, make configurable **/
            CPU_SET( d->core,
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
            pthread_yield();
            /** get to the timing, previous is captured by the lambda function **/
            uint64_t previous( 0 );
            /** begin assembly section to init previous **/
#ifdef   __x86_64
            __asm__ volatile(
#if RDTSCP  
             "\
               rdtscp                           \n\
               shl      $32, %%rdx              \n\
               orq      %%rax, %%rdx            \n\
               movq     %%rdx, %[prev]"
#elif defined INTEL
             "\
               mfence                           \n\
               rdtsc                            \n\
               shl      $32, %%rdx              \n\
               orq      %%rax, %%rdx            \n\
               movq     %%rdx, %[prev]"
#elif defined AMD
             "\
               lfence                           \n\
               rdtsc                            \n\
               shl      $32, %%rdx              \n\
               orq      %%rax, %%rdx            \n\
               movq     %%rdx, %[prev]"
#elif NOSERIAL
             "\
               rdtsc                            \n\
               shl      $32, %%rdx              \n\
               orq      %%rax, %%rdx            \n\
               movq     %%rdx, %[prev]"
#endif
               :
               /*outputs here*/
               [prev]    "=r" (previous)
               :
               /*inputs here*/
               :
               /*clobbered registers*/
               "rax","eax","rcx","ecx","rdx"
            );
#elif    __ARMEL__
#warning    Cycle counter not supported on this architecture
#elif    __ARMHF__
#warning    Cycle counter not supported on this architecture
#else
#warning    Cycle counter not supported on this architecture
#endif

            function = [&]( )
            {
               /** begin assembly sections **/
#ifdef   __x86_64
               uint64_t difference;
               __asm__ volatile(
#if RDTSCP               
               "\
               rdtscp                           \n\
               shl      $32, %%rdx              \n\
               orq      %%rax, %%rdx            \n\
               movq     %%rdx, %%rax            \n\
               movq     %[pre], %%rcx           \n\
               subq     %%rcx, %%rax            \n\
               movq     %%rax, %[diff]          \n\
               movq     %%rdx, %[prev]"
#elif AMD
               "\
               mfence                           \n\
               rdtsc                            \n\
               shl      $32, %%rdx              \n\
               orq      %%rax, %%rdx            \n\
               movq     %%rdx, %%rax            \n\
               movq     %[pre], %%rcx           \n\
               subq     %%rcx, %%rax            \n\
               movq     %%rax, %[diff]          \n\
               movq     %%rdx, %[prev]"

#elif INTEL
               "\
               lfence                           \n\
               rdtsc                            \n\
               shl      $32, %%rdx              \n\
               orq      %%rax, %%rdx            \n\
               movq     %%rdx, %%rax            \n\
               movq     %[pre], %%rcx           \n\
               subq     %%rcx, %%rax            \n\
               movq     %%rax, %[diff]          \n\
               movq     %%rdx, %[prev]"
#elif NOSERIAL
               "\
               rdtsc                            \n\
               shl      $32, %%rdx              \n\
               orq      %%rax, %%rdx            \n\
               movq     %%rdx, %%rax            \n\
               movq     %[pre], %%rcx           \n\
               subq     %%rcx, %%rax            \n\
               movq     %%rax, %[diff]          \n\
               movq     %%rdx, %[prev]"
#endif
                  :
                  /*outputs here*/
                  [diff]    "=r" ( difference ),
                  [prev]    "=r" ( previous   )
                  :
                  /*inputs here*/
                  [pre]    "m" ( previous   )
                  :
                  /*clobbered registers*/
                  "rax","eax","rcx","ecx","rdx"
               );
               /* convert to seconds for increment */
               const sclock_t seconds( (sclock_t) difference / 
                                       (sclock_t) frequency );

               clock->increment( seconds );
#elif    __ARMEL__
#warning Using dummy counter!!               
               clock->increment();
#elif    __ARMHF__
#warning Using dummy counter!!               
               clock->increment();
#else
#warning Using dummy counter!!               
               clock->increment();
#endif
            };
#else
#warning    Cycle counter currently supported for Linux only, defaulting to dummy counter!
            function = [&](){ clock->increment(); };
#endif
         }
         break;
         case( System ):
         {
#ifdef   __linux
            struct timespec curr_time;
            struct timespec prev_time;
            std::memset( &curr_time, 0, sizeof( struct timespec ) ); 
            std::memset( &prev_time, 0, sizeof( struct timespec ) ); 
            if( clock_gettime( CLOCK_REALTIME, &prev_time ) != 0 )
            {
               perror( "Failed to get initial time." );
            }
            function = [&]()
            {
               errno = 0;
               if( clock_gettime( CLOCK_REALTIME, &curr_time ) != 0 )
               {
                  perror( "Failed to get current time!" );
               }
               const struct timespec diff( 
                  { .tv_sec  = curr_time.tv_sec  - prev_time.tv_sec,
                    .tv_nsec = curr_time.tv_nsec - prev_time.tv_nsec } );
               prev_time = curr_time;
               /* update global time */
               const sclock_t seconds( 
                     (sclock_t ) diff.tv_sec + 
                        ( ( sclock_t ) diff.tv_nsec * 1.0e-9 ) );
               clock->increment( seconds );
            };
#elif defined __APPLE__
            uint64_t  previous( mach_absolute_time() );
            /** init **/
            static mach_timebase_info_data_t sTimebaseInfo;
            if( sTimebaseInfo.denom == 0 )
            {
               (void) mach_timebase_info( &sTimebaseInfo );
            }
            function = [&]()
            {
               const auto current( mach_absolute_time() );
               const auto diff( current - previous );
               previous = current;
               /** 
                * TODO, fix this, there's gotta be a better way
                * figure out what units the return val is in.
                */
                
                const uint64_t elapsedNano( diff * sTimebaseInfo.numer / 
                                                   sTimebaseInfo.denom );
                
                const sclock_t seconds( (sclock_t) elapsedNano * 1.0e-9 );
                clock->increment( seconds );
            };
#endif
         }
         break;
         default:
         {
            assert( false );
         }
         break;
      }
      d->setup = true;
      while( ! done )
      {
         function();
      }
      pthread_exit( nullptr );
   }

   pthread_t         updater;
};
#endif /* END _SYSTEMCLOCK_HPP_ */
