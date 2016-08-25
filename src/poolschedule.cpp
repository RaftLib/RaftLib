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
#include "sched_cmd_t.hpp"
#include <qthread/qthread.hpp>
#include <qthread/sinc.h>
#include <mutex>
#include <chrono>

#include "affinity.hpp"
#ifdef USE_PARTITION
#include "partition_scotch.hpp"
#endif
#include "defs.hpp"

pool_schedule::pool_schedule( raft::map &map ) : Schedule( map )
{
    //assert( qthread_init( 1 ) == QTHREAD_SUCCESS );
    assert( qthread_initialize() == QTHREAD_SUCCESS );
    thread_data_pool.reserve( kernel_set.size() );
}


pool_schedule::~pool_schedule()
{
    /** kill off the qthread structures **/
    qthread_finalize();
    /** delete thread data structs **/
    for( auto *td : thread_data_pool )
    {
        delete( td );
    }
}

void
pool_schedule::handleSchedule( raft::kernel * const kernel )
{
   //TODO implement me 
   UNUSED( kernel );
   assert( false );
}

void
pool_schedule::start()
{
    /** 
     * NOTE: this section is the same as the code in the "handleSchedule"
     * function so that it doesn't call the lock for the thread map.
     */
    auto &container( kernel_set.acquire() );
    for( auto * const k : container )
    {  
        auto *td( new thread_data( k ) );
        thread_data_pool.emplace_back( td );
        if( ! k->output.hasPorts() /** has no outputs, only 0 > inputs **/ )
        {
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
    bool keep_going( true );
    while( keep_going )
    {
        std::chrono::milliseconds dura( 3 );
        std::this_thread::sleep_for( dura );
        std::lock_guard< std::mutex > lock( tail_mutex );
        keep_going = false;
        for( auto * const td : tail )
        {
            if( ! td->finished  )
            {
                keep_going = true;
                break;
            }
        }
    }
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
      affinity::set( thread_d->loc );
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
