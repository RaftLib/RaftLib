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
              class RunContainer >
   static
   void
   simple( KernelList   &kernel_list, 
           RunContainer &run_container )
   {
      auto begin( run_container.cbegin() );
      auto it(    begin                  );
      auto end(   run_container.cend()   );
      for( auto * const kernel : kernel_list )
      {
         if( Schedule::isActive( kernel ) )
         {
            (*it)->addKernel( kernel );
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
