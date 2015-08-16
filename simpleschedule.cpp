/**
 * simpleschedule.cpp - 
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
#include <cassert>
#include <functional>
#include <iostream>

#include "kernel.hpp"
#include "map.hpp"
#include "simpleschedule.hpp"
#include "rafttypes.hpp"
#include "pthreadwrap.h"
simple_schedule::simple_schedule( Map &map ) : Schedule( map )
{
   pthread_mutex_init( &thread_map_mutex, nullptr );
}


simple_schedule::~simple_schedule()
{
   pthread_mutex_lock_d( &thread_map_mutex, __FILE__, __LINE__ );
   for( decltype( thread_map.size() ) index( 0 ); index < thread_map.size(); index++ )
   {
      delete( thread_map[ index ] );
   }
   pthread_mutex_unlock( &thread_map_mutex );
   pthread_mutex_destroy( &thread_map_mutex );
}


void
simple_schedule::start()
{
   auto &container( kernel_set.acquire() );
   for( auto * const k : container )
   {  
      auto *th_info( new thread_info_t() );
      /** set up data struct for threads **/
      th_info->data.k = k;
      th_info->data.finished = &(th_info->finished);
      pthread_create( &(th_info->th) /** thread **/, 
                      nullptr        /** no attributes **/, 
                      simple_run     /** function **/,
                      reinterpret_cast< void* >( &(th_info->data) ) );
      thread_map.emplace_back( th_info );
   }
   kernel_set.release();

   bool keep_going( true );
   while( keep_going )
   {
      pthread_mutex_lock_d( &thread_map_mutex, __FILE__, __LINE__ );
      keep_going = false;
      for( auto  *t_info : thread_map )
      {
         if( ! t_info->term )
         {
            if( t_info->finished )
            {
               /**
                * FIXME: the list could get huge for long running apps,
                * need to delete these entries...especially since we have
                * a lock on the list now 
                */
               pthread_join( t_info->th, nullptr );
               t_info->term = true;
            }
            else /* ! finished */
            {
               keep_going =  true;
            }
         }
      }
      pthread_mutex_unlock( &thread_map_mutex );
      sched_yield();
   }
   pthread_mutex_unlock( &thread_map_mutex );
   return;
}

void
simple_schedule::handleSchedule( raft::kernel * const kernel )
{
      auto *th_info( new thread_info_t() );
      /** 
       * thread function takes a reference back to the scheduler
       * accessible done boolean flag, essentially when the 
       * kernel is done, it can be rescheduled...and this
       * handles that.
       */
      th_info->data.k = kernel;
      th_info->data.finished = &(th_info->finished);
      pthread_create( &(th_info->th) /** thread **/, 
                      nullptr        /** no attributes **/, 
                      simple_run     /** function **/,
                      reinterpret_cast< void* >( &(th_info->data) ) );
      pthread_mutex_lock_d( &thread_map_mutex, __FILE__, __LINE__ );
      thread_map.emplace_back( th_info );
      pthread_mutex_unlock( &thread_map_mutex );
      thread_map.emplace_back( th_info );
      return;
}

void*
simple_schedule::simple_run( void * data ) 
{
   auto *thread_d( reinterpret_cast< thread_data* >( data ) );
   while( ! *(thread_d->finished) )
   {
      Schedule::kernelRun( thread_d->k, *(thread_d->finished) );
   }
   pthread_exit( nullptr );
}
