/**
 * raftthread.hpp - 
 * @author: Jonathan Beard
 * @version: Sat Jan 24 14:47:49 2015
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
#ifndef _RAFTTHREAD_HPP_
#define _RAFTTHREAD_HPP_  1
#include "thread_container.hpp"
#include "kernel.hpp"

class raft_thread
{
public:
   raft_thread();
   virtual ~raft_thread();


   void addKernel( raft::kernel *kernel );

   static void run( raft_thread &thread );

private:
   thread_container container;
};
#endif /* END _RAFTTHREAD_HPP_ */
