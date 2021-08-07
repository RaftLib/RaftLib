/**
 * poolschedule.cpp - 
 * @author: Jonathan Beard
 * @version: Thu Sep 11 15:49:57 2014
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
#ifdef USEQTHREADS 

#include <cassert>
#include <functional>
#include <iostream>
#include <algorithm>
#include <map>
#include <cassert>
#include "kernel.hpp"
#include "map.hpp"
#include "poolschedule.hpp"
#include "rafttypes.hpp"
#include <qthread/qthread.hpp>
#include <qthread/sinc.h>
#include <mutex>
#include <chrono>
#ifdef BENCHMARK
#include <sched.h>
#endif
#include <affinity>

#ifdef USE_PARTITION
#include "partition_scotch.hpp"
#endif
#include "defs.hpp"



pool_schedule::pool_schedule( raft::map &map ) : Schedule( map )
{
    const auto ret_val( qthread_initialize() );
    if( ret_val != 0 )
    {
        std::cerr << "failure to initialize qthreads runtime, exiting\n";
        exit( EXIT_FAILURE );
    }
    thread_data_pool.reserve( kernel_set.size() );
}


pool_schedule::~pool_schedule()
{
    /** kill off the qthread structures **/
    qthread_finalize();
}

void
pool_schedule::handleSchedule( raft::kernel * const kernel )
{
    auto *td( new thread_data( kernel ) );
    thread_data_mutex.lock();
    thread_data_pool.emplace_back( td );
    thread_data_mutex.unlock();
    if( ! kernel->output.hasPorts() /** has no outputs, only 0 > inputs **/ )
    {
        std::lock_guard< std::mutex > tail_lock( tail_mutex );
        /** destination kernel **/
        tail.emplace_back( td );
    }
    qthread_spawn( pool_schedule::pool_run,
                   (void*) td,
                   0,
                   0,
                   0,
                   nullptr,
                   NO_SHEPHERD,
                   0 );
    /** done **/
    return;
}

void
pool_schedule::start()
{
    auto &container( kernel_set.acquire() );
    for( auto * const k : container )
    {  
        (this)->handleSchedule( k );
    }
    /**
     * NOTE: can't quite get the sync object behavior to work 
     * quite well enough for this application. Should theoretically
     * work according to the documentation here:
     * http://www.cs.sandia.gov/qthreads/man/qthread_spawn.html
     * however it seems that the wait segfaults. Adding on the
     * TODO list I'll implement a better mwait monitor vs. spinning
     * which is relatively bad.
     */
    kernel_set.release();
START:        
    std::chrono::milliseconds dura( 3 );
    std::this_thread::sleep_for( dura );
    tail_mutex.lock();
    for( auto * const td : tail )
    {
        if( ! td->finished  )
        {
            tail_mutex.unlock();
            goto START;
        }
    }
    tail_mutex.unlock();
    return;
}

aligned_t pool_schedule::pool_run( void *data )
{
   assert( data != nullptr );
   auto * const thread_d( reinterpret_cast< thread_data* >( data ) );
   ptr_map_t in;
   ptr_set_t out;
   ptr_set_t peekset;

   Schedule::setPtrSets( thread_d->k, 
                        &in, 
                        &out,
                        &peekset );
#if 0 //figure out pinning later                        
   if( thread_d->loc != -1 )
   {
      /** call does nothing if not available **/
      raft::affinity::set( thread_d->loc );
   }
   else
   {
#ifdef USE_PARTITION
       assert( false );
#endif
   }
#endif   
   volatile bool done( false );
   std::uint8_t run_count( 0 );
#ifdef BENCHMARK
   raft::kernel::initialized_count++;
   while( raft::kernel::initialized_count != raft::kernel::kernel_count )
   {
       raft::yield();
   }
#endif 
   while( ! done )
   {
      Schedule::kernelRun( thread_d->k, done );
      //FIXME: add back in SystemClock user space timer
      //set up one cache line per thread
      if( run_count++ == 20 || done )
      {
        run_count = 0;
        //takes care of peekset clearing too
        Schedule::fifo_gc( &in, &out, &peekset );
        qthread_yield();
      }
   }
   thread_d->finished = true;
   return( 1 );
}

#endif
