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
   /** start up N threads **/
   const auto n_threads( std::thread::hardware_concurrency() );
   for( auto index( 0 ); index < n_threads; index++ )
   {
      container.push_back( new KernelContainer() );
      pool.push_back( new std::thread( poolschedule::poolrun, 
                                       std::ref( (*container[ index ]) ),
                                       std::ref( 
   }
}


pool_schedule::~pool_schedule()
{
}

bool
pool_schedule::scheduleKernel( raft::kernel *kernel )
{
}

void
pool_schedule::start()
{
   
}
