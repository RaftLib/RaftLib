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
#include <set>
#include <thread>
#include <cstdint>
#include "schedule.hpp"
#include "kernelcontainer.hpp"

namespace raft{
   class kernel;
   class map;
}


class pool_schedule : public Schedule
{
public:
   /**
    * pool_schedule - constructor, takes a map object, 
    * calling this will launch threads.  scheduler itself
    * is also run as a thread.
    * @param   map - raft::map&
    */
   pool_schedule( raft::map &map );

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
   /** BEGIN FUNCTIONS **/
   /**
    * scheduleKernel - override base class function in order
    * to add kernels to the right place.
    * @param kernel - raft::kernel*
    * @return bool - always true
    */
   virtual bool scheduleKernel( raft::kernel * const kernel );

   /**
    * container_min - returns true if the input queue of a has
    * fewer items than the input queue of b
    * @param a - kernel_container * const
    * @param b - kernel_container * const
    * @return  bool - true if a->qsize() < b->qsize()
    */
   static bool  container_min_input( kernel_container * const a,
                                     kernel_container * const b );

   /**
    * container_max - returns true if the output queue of a
    * is greater than b.
    * @param   a - kernel_container * const
    * @param   b - kernel_container * const
    * @return  bool - true if a->outqsize > b->qoutsize
    */
   static bool  container_min_output( kernel_container * const a,
                                      kernel_container * const b );


   /** END FUNCTIONS, BEGIN VAR DECLS **/
   /**
    * The thread has to have this much more "work" than 
    * the previous thread in order to get moved ot a new
    * thread.  Used in pool_schedule::start().
    */
   const float diff_weight = static_cast< const float >( .20 );
   /**
    * total # of hardware supported threads 
    */
   const decltype( std::thread::hardware_concurrency() )    n_threads;
   /**
    * used as a thread pool
    */
   std::vector< std::thread* >      pool;
   /** 
    * max_heap_container - sorted by max output-queue occupancy 
    */
   std::vector< kernel_container* > container;

   std::size_t                      kernel_count = 0;
   std::vector< kernel_container* >::iterator      container_it;
};
#endif /* END _POOLSSCHEDULE_HPP_ */
