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

simple_schedule::simple_schedule( Map &map ) : Schedule( map )
{
   /** nothing to do here **/
}


simple_schedule::~simple_schedule()
{
   /** note: kernels are deleted by the map! **/
}

bool
simple_schedule::scheduleKernel( raft::kernel *kernel )
{
   assert( kernel != nullptr );
   kernel_map.push_back( kernel ); 
   return( true );
}

void
simple_schedule::start()
{
   struct thread_info_t
   {
      std::thread *th      = nullptr;
      bool        finished = false;
   };

   std::vector< thread_info_t > thread_map( kernel_map.size() );
   
   for( std::size_t index( 0 ); index < kernel_map.size(); index++ )
   {
      auto bound_func = [&]( raft::kernel *kernel, bool &finished ){
         auto sig_status( raft::proceed );
         while( sig_status == raft::proceed )
         {
            if( kernelHasInputData( kernel ) )
            {
               sig_status = kernel->run();
            }
            else if( kernelHasNoInputPorts( kernel ) /** no data too **/ )
            {
               sig_status = raft::stop;
            }
            else
            {
               std::this_thread::yield();
            }
         }
         /** invalidate output queues **/
        invalidateOutputPorts( kernel );
         finished = true;
      };
      thread_map[ index ].th = 
         new std::thread( bound_func                               /* kernel loop */, 
                          kernel_map[ index ]                      /* kernel ptr  */,
                          std::ref( thread_map[ index ].finished ) /* finished ref */);
   }

   bool keep_going( true );
   while( keep_going )
   {
      keep_going = false;
      for( auto  &t_info : thread_map )
      {
         if( t_info.th != nullptr )
         {
            if( t_info.finished )
            {
               t_info.th->join();
               delete( t_info.th );
               t_info.th = nullptr;
            }
            else /* ! finished */
            {
               keep_going =  true;
            }
         }
      }
   }
}
