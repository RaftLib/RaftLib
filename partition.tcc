/**
 * partition.tcc - 
 * @author: Jonathan Beard
 * @version: Fri Mar 20 08:53:12 2015
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
#ifndef _PARTITION_TCC_
#define _PARTITION_TCC_  1
#include <cstdint>
#include <cstddef>
#include "schedule.hpp"
#include <utility>

class partition
{
public:
   /**
    * simple - partiion the kernels in kernel_list
    * using a simple partitioning scheme evenly as
    * possible amongst the given run_containers 
    * in run_container.
    * @param   kernel_list - C&
    * @param   run_container - R&
    */
   template < class KernelList,
              class RunContainer,
              class PriorMapping >
   static
   void
   simple( KernelList   &kernel_list, 
           RunContainer &run_container,
           PriorMapping &prior )
   {
      auto begin( run_container.cbegin() );
      auto it(    begin                  );
      auto end(   run_container.cend()   );
      for( auto * const kernel : kernel_list )
      {
         if( Schedule::isActive( kernel ) )
         {
            /** unschedule kernel **/
            auto prior_container( prior.find( kernel ) );
            if( prior_container != prior.end() )
            {
               auto * const prior_buff( (*prior_container).second->buff );
               auto &unschedule( prior_buff->template allocate< sched_cmd_t >() );
               unschedule.cmd    = schedule::REMOVE;
               unschedule.kernel = kernel;
               prior_buff->send();
               prior.erase( prior_container );
            }
            /** add kernel to new container **/
            auto * const buff( (*it)->buff );
            auto &to_schedule( buff->template allocate< sched_cmd_t >() );
            to_schedule.cmd    = schedule::ADD;
            to_schedule.kernel = kernel;
            buff->send();
            /** insert new kernel -> container map **/
            prior.insert( std::make_pair( kernel, (*it) ) );
            if( ++it == end ) 
            {
               it = begin;
            }
         }
      }
      return;
   }
};

#endif /* END _PARTITION_TCC_ */
