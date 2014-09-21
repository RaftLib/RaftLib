/**
 * schedule.hpp - 
 * @author: Jonathan Beard
 * @version: Thu Sep 11 15:42:28 2014
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
#ifndef _SCHEDULE_HPP_
#define _SCHEDULE_HPP_  1
#include "map.hpp"
#include "kernel.hpp"

class Schedule
{
public:
   
   Schedule( Map &map );
   virtual ~Schedule();
   
   /**
    * start - called to start execution of all
    * kernels.  Implementation specific so it
    * is purely virtual.
    */
   virtual void start() = 0;

protected:
   
   
   /**
    * scheduleKernel - adds the kernel "kernel" to the
    * schedule, ensures that it is run.  Other than
    * that there are no guarantees for its execution.
    * It is purely virtual in its implementation.
    * @param kernel - Kernel*
    * @return  bool  - returns false if the kernel is
    * already scheduled.
    */
   virtual bool scheduleKernel( Kernel *kernel ) = 0;
   
   /**
    * start_func - calls kernel run functions,
    * should be implemented by individual scheduler
    * since each one will have slightly differing
    * implementations.  The data struct is to be
    * cast to some internal representation if desired
    * or left as a nullptr.
    * @param   kernel - Kernel*
    * @param   data   - void*
    */
   virtual void start_func( Kernel *kernel, void *data ) = 0;
};
#endif /* END _SCHEDULE_HPP_ */
