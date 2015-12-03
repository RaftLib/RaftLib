/**
 * noparallel.hpp - dummy package to remove auto-parallelization 
 * @author: Jonathan Beard
 * @version: Mon Aug 10 20:00:25 2015
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
#ifndef _NOPARALLEL_HPP_
#define _NOPARALLEL_HPP_  1


#include "kernel.hpp"
#include "port_info.hpp"
#include "fifo.hpp"
#include "allocate.hpp"
#include "schedule.hpp"
#include <set>
#include <iostream>

class Map;



class no_parallel
{
public:
   no_parallel( Map &map, 
                   Allocate &alloc,
                   Schedule &sched,
                   volatile bool &exit_para );

   virtual ~no_parallel() = default;
   virtual void start();

};

#endif /* END _NOPARALLEL_HPP_ */
