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
#include <cassert>
#include <functional>
#include <iostream>
#include <algorithm>
#include <chrono>
#include <map>
#include <thread>
#include "kernel.hpp"
#include "map.hpp"
#include "poolschedule.hpp"
#include "rafttypes.hpp"

#include "partition.tcc"

pool_schedule::pool_schedule( Map &map ) : Schedule( map ),
                                     n_threads( std::thread::hardware_concurrency() ),
                                     pool( n_threads ),
                                     container( n_threads ),
                                     status_flags( n_threads )
{
   for( std::int64_t index( 0 ); index < n_threads; index++ )
   {
      status_flags[index] = false;
      container[ index ]  = new container_struct();
      pool[ index ]       = new std::thread( poolrun,
                                             container[ index ],
                                             std::ref(  status_flags[index] ) );
   }
}


pool_schedule::~pool_schedule()
{
   for( auto *th : pool )
   {
      delete( th );
   }
   for( auto *kc : container )
   {
      delete( kc );
   }
}

bool
pool_schedule::scheduleKernel( raft::kernel *kernel )
{
   assert( kernel != nullptr );
   kernel_map.emplace_back( kernel );
   return( true );
}

void
pool_schedule::start()
{
   std::map< raft::kernel*,
             container_struct* >
   curr_mapping;
              
   partition::simple( kernel_map,
                      container,
                      curr_mapping );
   
   auto is_done( []( std::vector< container_struct* > &containers ) -> bool
   {
      for( auto * const c : containers )
      {
         if( c->kernel_container->size() != 0 || c->buff->size() != 0 )
         {
            return( false );
         }
      }
      return( true );
   } );

   while( ! is_done( container ) )
   {
      const std::chrono::milliseconds dura( 100 );
      std::this_thread::sleep_for( dura );
#if 0
      partition::simple( kernel_map,
                         container,
                         curr_mapping );
#endif
   }
   /** done **/
   for( auto &flag : status_flags )
   {
      flag = 1;
   }
   for( std::thread *thr : pool )
   {
      thr->join();
   }
}

void 
pool_schedule::poolrun( container_struct *container, volatile std::uint8_t &sched_done )
{
   while( sched_done == 0 )
   {
      /** two lists for actions **/ 
      std::set< raft::kernel* > unschedule_list;
      /** buff **/
      auto *buff( container->buff );
      while( buff->size() > 0 )
      {
         auto &cmd_struct( buff->peek< sched_cmd_t >() );
         switch( cmd_struct.cmd )
         {  
            case( schedule::ADD ):
            {
               /** add kernel to run container **/
               container->kernel_container->insert( cmd_struct.kernel );
            }
            break;
            case( schedule::REMOVE ):
            {
               /** add to unschedule list, reconcile at end of run **/
               unschedule_list.insert( cmd_struct.kernel );
            }
            break;
            default:
               assert( false );
         }
         /** clean up buffer **/
         buff->unpeek();
         buff->recycle( 1 );
      }
      /** run kernels **/
      auto * const list( container->kernel_container );
      for( auto * const kernel : *list )
      {
         bool done( false );
         Schedule::kernelRun( kernel, done );
         if( done )
         {
            unschedule_list.insert( kernel );
         }
      }
      /** reconcile all kernels to remove **/
      for( raft::kernel * const kernel : unschedule_list )
      {
         auto el( list->find( kernel ) );
         assert( el != list->end() );
         list->erase( el );
         Schedule::inactivate( kernel );
      }
   }
}
