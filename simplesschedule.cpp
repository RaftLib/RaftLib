/**
 * simplesschedule.cpp - 
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
#include "simplesschedule.hpp"

SimpleSchedule::SimpleSchedule() : Schedule()
{
}


SimpleSchedule::~SimpleSchedule()
{
   for( Kernel *kern : kernel_map )
   {
      if( kern != nullptr )
      {
         delete( kern );
      }
   }
}


SimpleSchedule::addKernel( Kernel *kernel )
{
   assert( kernel != nullptr );
   kernel_map.push_back( kernel ); 
}


SimpleSchedule::start()
{
   std::vector< Kernel* > thread_map( kernel_map.size() );
   for( Kernel *kern : kernel_map )
   {
      auto bound_func = std::bind( run_func, kern );
      thread_map.push_back( new std::thread( bound_func ) );
   }
   for( std::thread *thread : thread_map )
   {
      thread->join();
      delete( thread );
   }
}
