/**
 * simpleschedule.hpp - 
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
#ifndef _SIMPLESSCHEDULE_HPP_
#define _SIMPLESSCHEDULE_HPP_  1
#include <vector>

class Map;
namespace raft{
   class kernel;
}

class simple_schedule : public Schedule
{
public:
   simple_schedule( Map &map );

   virtual ~simple_schedule();

   virtual void start(); 
   
protected:
   
   virtual bool scheduleKernel( raft::kernel *kernel );

   /**
    * NOTE: for more complex schedules it'd behoove us
    * to keep more information here for each scheduled
    * kernel.
    */
   std::vector< raft::kernel* > kernel_map;
};
#endif /* END _SIMPLESSCHEDULE_HPP_ */
