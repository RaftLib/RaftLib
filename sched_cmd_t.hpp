/**
 * sched_cmd_t.hpp - 
 * @author: Jonathan Beard
 * @version: Sat Mar 21 11:05:19 2015
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
#ifndef _SCHED_CMD_T_HPP_
#define _SCHED_CMD_T_HPP_  1
#include <cstdint>

/** full def in kernel.hpp **/
namespace raft
{
   class kernel;
}

namespace schedule{
   enum sched_cmd : std::int8_t { ADD, REMOVE };
}

struct sched_cmd_t
{
   sched_cmd_t( const schedule::sched_cmd cmd,
                raft::kernel *kernel ) : cmd( cmd ),
                                         kernel( kernel )
   {
   }

   sched_cmd_t( const sched_cmd_t &other ) : cmd( other.cmd ),
                                             kernel( other.kernel )
   {

   }

   virtual ~sched_cmd_t() = default;
   schedule::sched_cmd      cmd;
   raft::kernel *kernel = nullptr;
};

#endif /* END _SCHED_CMD_T_HPP_ */
