/**
 * kernelcontainer.hpp - 
 * @author: Jonathan Beard
 * @version: Wed Jan 14 08:12:47 2015
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
#include <mutex>
#include <vector>
#include "kernel.hpp"
class KernelContainer
{

public:
   KernelContainer();
   virtual ~KernelContainer();

   auto begin() -> decltype( list.begin() );
   auto end() -> decltype( list.end() );
   
private:
   std::mutex                   mutex;
   std::vector< raft::kernel *> list;
};
#endif /* END _KERNELCONTAINER_HPP_ */
