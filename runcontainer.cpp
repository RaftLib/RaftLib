/**
 * runcontainer.cpp - 
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
#include <cassert>
#include <thread>
#include <mutex>

#include "runcontainer.hpp"
#include "kernel.hpp"

void
runcontainer::addKernel( raft::kernel *kernel )
{
   assert( kernel != nullptr );
   thread_work.mutex.lock();
   thread_work.list.push_back( kernel );
   thread_work.mutex.unlock();
   return;
}

void
runcontainer::start()
{
   thread_work.mutex.lock();
   
   thread_work.mutex.unlock();
   return;
}
