/**
 * noparallel.cpp -
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
#include "noparallel.hpp"
#include "map.hpp"
#include "common.hpp"
#include "streamingstat.tcc"
#include <map>

no_parallel::no_parallel(       raft::map &map,
                                Allocate &alloc,
                                Schedule &sched,
                                volatile bool &exit_para )
{
   (void) map;
   (void) alloc;
   (void) sched;
   (void) exit_para;

   /** nothing to do here, move along **/
}


void
no_parallel::start()
{
   return;
}
