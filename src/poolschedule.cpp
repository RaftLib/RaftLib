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

#include "affinity.hpp"
#ifdef USE_PARTITION
#include "partition_scotch.hpp"
#endif
#include "defs.hpp"

pool_schedule::pool_schedule( raft::map &map ) : Schedule( map )
{
    assert( qthread_initialize() == QTHREAD_SUCCESS );
    return_flags.reserve( dst_kernels.size() );
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
            return_flags.emplace_back( 0 );
            qthread_fork(  pool_schedule::pool_run              /** function **/,
                           (void*) td                           /** data **/,
                           (aligned_t*) &return_flags.back()    /** ptr to aligned_t flag **/);

        }
        else
        {
            /** else **/
            qthread_fork(  pool_schedule::pool_run              /** function **/,
                           (void*) td                           /** data **/,
                           (aligned_t*) 0 /** no flag **/ ); 
        }
    }
    kernel_set.release();
    /** now we just need to loop through and wait for the flags to be valid **/
    for( auto &val : return_flags )
    {
        qthread_readFF( nullptr, (aligned_t*)&val );
    }
    /** logically if we exit these blocking functions then we're clear, exit **/
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
   while( ! thread_d->finished )
   {
      Schedule::kernelRun( thread_d->k, thread_d->finished );
      //takes care of peekset clearing too
      Schedule::fifo_gc( &in, &out, &peekset );
   }
   return( 1 );
}

#endif
