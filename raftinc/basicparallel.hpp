/**
 * basicparallel.hpp - 
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
#ifndef BASICPARALLEL_HPP
#define BASICPARALLEL_HPP  1


#include "kernel.hpp"
#include "port_info.hpp"
#include "fifo.hpp"
#include "allocate.hpp"
#include "schedule.hpp"
#include <set>
#include <iostream>

namespace raft
{
    class map;
}

/** right now we're only considering single input, single output kernels **/
struct stats
{
   std::uint16_t occ_in  = 0;
   float        service_rate = static_cast< float >( 0 );
   friend   std::ostream& operator << ( std::ostream &stream, stats &s )
   {
      stream << "occupancy count: " << s.occ_in << "\n";
      stream << "service rate: " << s.service_rate << "\n";
      return( stream );
   }
};


class basic_parallel
{
public:
   basic_parallel( raft::map    &map, 
                   Allocate     &alloc,
                   Schedule     &sched,
                   bool         &exit_para );

   virtual ~basic_parallel() = default;
   virtual void start();

   
protected:
   /** both convenience structs, hold exactly what the names say **/
   kernelkeeper   &source_kernels;
   kernelkeeper   &all_kernels;
   Allocate       &alloc;
   Schedule       &sched;
   bool           &exit_para;
};

#endif /* END BASICPARALLEL_HPP */
