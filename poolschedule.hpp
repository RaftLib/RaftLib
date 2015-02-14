/**
 * poolschedule.hpp - 
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
#ifndef _POOLSSCHEDULE_HPP_
#define _POOLSSCHEDULE_HPP_  1
#include <vector>
#include <thread>
#include "runcontainer.hpp"

class Map;
namespace raft{
   class kernel;
}

class pool_schedule : public Schedule
{
public:
   pool_schedule( Map &map );

   virtual ~pool_schedule();

   virtual void start(); 
   
protected:
   const decltype( std::thread::hardware_concurrency() )    n_threads;
   std::vector< std::thread* >     pool;
   std::vector< KernelContainer* > container;
   std::vector< bool >             status_flags;

   std::vector< raft::kernel* >    kernel_map;

private:
   static void poolrun( KernelContainer &container, volatile bool &sched_done );
};
#endif /* END _POOLSSCHEDULE_HPP_ */
