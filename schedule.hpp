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
#include "kernel.hpp"
class Schedule
{
public:
   Schedule()           = default;
   virtual ~Schedule()  = default;

   virtual void addKernel( Kernel *kernel ) = 0;
   virtual void start() = 0;
protected:
   const auto run_func = []( Kernel *kernel ) -> void
   {
      kernel->run();
      return;
   };
};
#endif /* END _SCHEDULE_HPP_ */
