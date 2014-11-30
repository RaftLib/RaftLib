/**
 * runcontainer.hpp - 
 * @author: Jonathan Beard
 * @version: Wed Nov 26 08:28:49 2014
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
#ifndef _RUNCONTAINER_HPP_
#define _RUNCONTAINER_HPP_  1
#include <mutex>
#include <vector>

namespace raft{
   class kernel;
}

class runcontainer
{
public:
   runcontainer();
   virtual ~runcontainer();
   /**
    * addKernel - add a kernel to this thread
    * @param - kernel - raft::kernel*, must not be nullptr
    */
   void  addKernel( raft::kernel *kernel );

   /**
    * start - similiar to the simpleschedule start(), but 
    * contains the logic for only a single thread.
    */
   void start();
   

protected:
   struct {
      /** 
       * mutex to be locked before accessing list
       */
      std::mutex                          mutex;
      /**
       * vector to contain all kernels to be run
       */
      std::vector< raft::kernel *kernel > list;
   } thread_work;
};
#endif /* END _RUNCONTAINER_HPP_ */
