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

#include "kernel.hpp"
#include "map.hpp"
#include "poolschedule.hpp"
#include "rafttypes.hpp"

pool_schedule::pool_schedule( Map &map ) : Schedule( map ),
                    thread_pool( std::thread::hardware_concurrency() )
{
   /** nothing to do here **/
   
}


pool_schedule::~pool_schedule()
{
   /** note: kernels are deleted by the map! **/
   /** delete thread pool **/
   for( auto *thread : thread_pool )
   {
      delete( thread );
   }
}

bool
pool_schedule::scheduleKernel( raft::kernel *kernel )
{
   assert( kernel != nullptr );
   kernel_map.push_back( kernel ); 
   return( true );
}

void
pool_schedule::start()
{

   
    
   
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
   return;
}
