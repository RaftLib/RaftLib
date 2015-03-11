/**
 * poolschedule.hpp - scheduler starts up the maximum number
 * of threads supported in hardware for the currently executing
 * node, then awaits kernels to be mapped to it by the mapper. 
 * Once mapped, the scheduler continues to execute each kernel
 * in a round-robin fashion until execution is complete.
 *
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
#include <cstdint>
#include "schedule.hpp"
#include "kernelcontainer.hpp"

class Map;
namespace raft{
   class kernel;
}


class pool_schedule : public Schedule
{
public:
   /**
    * pool_schedule - constructor, takes a map object, 
    * calling this will launch threads.  scheduler itself
    * is also run as a thread.
    * @param   map - Map&
    */
   pool_schedule( Map &map );

   /**
    * destructor, deletes threads and cleans up container
    * objects.
    */
   virtual ~pool_schedule();

   /**
    * start - call to start executing map, at this point
    * the mapper sould have checked the topology and 
    * everything should be set up for running.
    */
   virtual void start(); 
   
protected:
   /**
    * scheduleKernel - override base class function in order
    * to add kernels to the right place.
    * @param kernel - raft::kernel*
    * @return bool - always true
    */
   virtual bool scheduleKernel( raft::kernel *kernel );

   /**
    * total # of hardware supported threads 
    */
   const decltype( std::thread::hardware_concurrency() )    n_threads;
   /**
    * used as a thread pool
    */
   std::vector< std::thread* >     pool;
   /**
    * container - holds all the kernels
    */
   std::vector< KernelContainer* > container;
   /**
    * used std::uint8_t b/c I don't want a bitset from std::vector, 
    * status_flags used to tell the sub-schedulers when to quit.
    */
   std::vector< std::uint8_t >             status_flags;

   /**
    * stores all kernels that we're currently executing, might
    * be removed from KernelContainer objects, but they'll still
    * be here. TODO, double check to make sure we're deleting
    * and removing these once everything is done.
    */
   std::vector< raft::kernel* >    kernel_map;

private:
   /**
    * poolrun- called by each thread within 
    * the pool, essentially this function acts as a 
    * mini-scheduler which runs all kernels in a given
    * container.
    */
   static void poolrun( KernelContainer *container, 
                        volatile std::uint8_t &sched_done );
};
#endif /* END _POOLSSCHEDULE_HPP_ */
