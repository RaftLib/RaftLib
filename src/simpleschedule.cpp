/**
 * simpleschedule.cpp - 
 * @author: Jonathan Beard
 * @version: Thu Sep 11 15:49:57 2014
 * 
 * Copyright 2020 Jonathan Beard
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
#include <cassert>
#include <functional>
#include <iostream>
#include <map>
#include <cmath>
#include <chrono>
#include <affinity>

#include "kernel.hpp"
#include "map.hpp"
#include "simpleschedule.hpp"
#include "rafttypes.hpp"

#ifdef USE_PARTITION
#include "partition_scotch.hpp"
#endif
#include "defs.hpp"

#ifdef STATIC_CORE_ASSIGN
/**
 * this is really just for benchmarking/debug
 * purposes, the map is indexed on the memory
 * address of each kernel, the second param
 * is the core to assign to.
 */
extern std::map< std::uintptr_t, int > *core_assign;
#endif

simple_schedule::simple_schedule( raft::map &map ) : Schedule( map )
{
}


simple_schedule::~simple_schedule()
{
   std::lock_guard<std::mutex> guard( thread_map_mutex );
   for( auto *th_info : thread_map )
   {
      delete( th_info );
      th_info = nullptr;
   }
}


void
simple_schedule::start()
{
   /** 
    * NOTE: this section is the same as the code in the "handleSchedule"
    * function so that it doesn't call the lock for the thread map.
    */
   auto &container( kernel_set.acquire() );
   for( auto * const k : container )
   {  
      handleSchedule( k );
   }
   kernel_set.release();
   
   bool keep_going( true );
   while( keep_going )
   {
      while( ! thread_map_mutex.try_lock() )
      {
         std::this_thread::yield();
      }     
      //exit, we have a lock
      keep_going = false;
      for( auto  *t_info : thread_map )
      {
         if( ! t_info->term )
         {
            //loop over each thread and check if done
            if( t_info->finished )
            {
               /**
                * FIXME: the list could get huge for long running apps,
                * need to delete these entries...especially since we have
                * a lock on the list now 
                */
               t_info->th.join();
               t_info->term = true;
            }
            else /* a kernel ! finished */
            {
               keep_going =  true;
            }
         }
      }
      //if we're here we have a lock and need to unlock
      thread_map_mutex.unlock();
      /**
       * NOTE: added to keep from having to unlock these so frequently
       * might need to make the interval adjustable dep. on app
       */
      std::chrono::milliseconds dura( 3 );
      std::this_thread::sleep_for( dura );
   }
   return;
}

void
simple_schedule::handleSchedule( raft::kernel * const kernel )
{
      /** 
       * TODO: lets add the affinity dynamically here
       */
      auto * const th_info( new thread_info_t( kernel ) );
      th_info->data.loc = kernel->getCoreAssignment();

      /** 
       * thread function takes a reference back to the scheduler
       * accessible done boolean flag, essentially when the 
       * kernel is done, it can be rescheduled...and this
       * handles that.
       */
      while( ! thread_map_mutex.try_lock() )
      {
         std::this_thread::yield();
      }
      thread_map.emplace_back( th_info );
      /** we got here, unlock **/
      thread_map_mutex.unlock();
      return;
}

void
simple_schedule::simple_run( void * data ) 
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
   while( ! *(thread_d->finished) )
   {
      bool validScheduling = Schedule::kernelRun( thread_d->k, *(thread_d->finished) );
      //takes care of peekset clearing too
      Schedule::fifo_gc( &in, &out, &peekset );

      if(validScheduling == false)
      {
        std::chrono::milliseconds dura( 5 );
        std::this_thread::sleep_for( dura );
      }
   }
}
