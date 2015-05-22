/**
 * kernelcontainer.hpp - 
 * @author: Jonathan Beard
 * @version: Sun Mar 22 09:13:32 2015
 * 
 * Copyright 2015 Jonathan Beard
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
#ifndef _KERNELCONTAINER_HPP_
#define _KERNELCONTAINER_HPP_  1
#include <queue>
#include <type_traits>
#include "sched_cmd_t.hpp"
#include "ringbuffer.tcc"
#include "ringbuffertypes.hpp"

namespace raft{
   class kernel;
}

class kernel_container
{
public:
   
   using buffer = RingBuffer< sched_cmd_t                /** type                **/, 
                              Type::RingBufferType::Heap /** heap alloc          **/, 
                              false                      /** no instrumentation  **/ >;
   /**
    * kernel_container - default constructor, initializes all above pointers.
    */
   kernel_container();
   
   /**
    * kernel_container - constructor, initializes all above pointers.
    * @param N - const std::size_t, default size of buffer
    */
   kernel_container( const std::size_t N );

   /** 
    * default destructor, cleans up all pointers
    */
   ~kernel_container();

   /**
    * getInput - get input FIFO
    * @return buffer&
    */
   buffer& getInputQueue();

   /**
    * getOutputQueue
    * @return buffer&
    */
   buffer& getOutputQueue();
   
   /**
    * size - returns the number of items currently scheduled
    * for this container.
    * @return std::size_t, number of items
    */
   std::size_t size();

   /**
    * container_run - function to be used by a thread which 
    * is called until the appropriate signal is sent (defined
    * in sched_cmd_t.hpp.
    * @param   container - kernel_container&
    */
   static void container_run( kernel_container &container  );
private:
   using kernel_container_t = std::queue< raft::kernel* >;
   kernel_container_t  preempted_kernel_pool;
   buffer             *input_buff   = nullptr; 
   buffer             *output_buff  = nullptr;         
};


#endif /* END _KERNELCONTAINER_HPP_ */
