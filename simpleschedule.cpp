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


void
simple_schedule::start()
{
   struct thread_info_t
   {
      std::thread *th      = nullptr;
      bool        finished = false;
   };

   std::vector< thread_info_t > thread_map( kernel_set.size() );
   

   std::size_t index( 0 );
   for( auto * const k : kernel_set )
   {
      thread_map[ index ].th = 
         new std::thread( simple_run, 
                          k,
                          std::ref( thread_map[ index ].finished ) /* finished ref */);
      index++;
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

void
simple_schedule::simple_run( raft::kernel * const kernel,
                             volatile bool        &finished )
{
   while( ! finished )
   {
      Schedule::kernelRun( kernel, finished );
   }
}
