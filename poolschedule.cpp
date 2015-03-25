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
#include "utility.hpp"
#include "sched_cmd_t.hpp"

pool_schedule::pool_schedule( Map &map ) : Schedule( map ),
                                     n_threads( std::thread::hardware_concurrency() ),
                                     pool( n_threads ),
                                     container( n_threads )
{
   for( int i( 0 ); i < n_threads; i++ )
   {
      /** initialize container objects **/
      container[ i ] = new kernel_container();
      /** initialize threads **/
      pool[ i ] = new std::thread( kernel_container::container_run,
                                   std::ref( *(container[ i ]) ) );
   }
}


pool_schedule::~pool_schedule()
{
   assert( kernel_count == 0 );
   /** join threads **/
   for( std::thread *thr : pool )
   {
      thr->join();
   }
   /** delete threads **/
   for( auto *th : pool )
   {
      delete( th );
   }
   /** delete containers **/
   for( auto *c : container )
   {
      delete( c );
   }
}

bool
pool_schedule::scheduleKernel( raft::kernel * const kernel )
{
   assert( kernel != nullptr );
   /** get a container **/
   auto it( std::min_element( container.begin(), 
                              container.end(),
                              container_min_input ) );
   auto &buffer( (*it)->getInputQueue() );
   auto &new_cmd( buffer.allocate< sched_cmd_t >() );
   new_cmd.cmd    = schedule::add;
   new_cmd.kernel = kernel;
   buffer.send();
   kernel_count++;
   return( true );
}


void
pool_schedule::start()
{
   while( kernel_count > 0 )
   {
      auto it( std::max_element( container.begin(),
                                 container.end(),
                                 container_min_output ) );
       
      /** we want to get the max queue occupancy **/
      auto &in_buff( (*it)->getOutputQueue() );
      if( in_buff.size() > 0 )
      {
         /** some message exists **/
         auto &rcvd_cmd( in_buff.peek< sched_cmd_t >() );
         switch( rcvd_cmd.cmd )
         {
            case( schedule::reschedule ):
            {
               auto it_r( std::min_element( container.begin(), 
                                            container.end(),
                                            container_min_input ) );
               const auto container_to_use
               ( ( (*it_r)->getInputQueue().size() <
                        (*it)->getInputQueue().size() * diff_weight ) ? it_r : it );
               
               auto &buffer( (*container_to_use)->getInputQueue() );
               auto &send_cmd( buffer.allocate< sched_cmd_t >() );
               send_cmd.cmd    = schedule::add;
               send_cmd.kernel = rcvd_cmd.kernel;
               buffer.send();
            }
            break;
            case( schedule::kernelfinished ):
            {
               /** remove kernel **/
               kernel_count--;
            }
            break;
            default:
            {
               std::cerr << "Invalid signal: " << 
                  schedule::sched_cmd_str[ rcvd_cmd.cmd ] << "\n";
               exit( EXIT_FAILURE );
            }
         }
         in_buff.unpeek();
         in_buff.recycle( 1 );
      }
   }

   /** all done, shutdown **/ 
   for( auto * const c : container )
   {
      auto &out_buff( c->getInputQueue() );
      auto &new_cmd( out_buff.allocate< sched_cmd_t >() );
      new_cmd.cmd    = schedule::shutdown;
      new_cmd.kernel = nullptr;
      out_buff.send();
   }
}

bool
pool_schedule::container_min_input( kernel_container * const a,
                                    kernel_container * const b )
{
   return( a->getInputQueue().size() < b->getInputQueue().size() ? true : false ); 
}

bool
pool_schedule::container_min_output( kernel_container * const a,
                                     kernel_container * const b )
{
   return( a->getOutputQueue().size() < b->getOutputQueue().size() ? true : false );
}
