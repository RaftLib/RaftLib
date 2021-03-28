/**
 * portmap_t.hpp - 
 * @author: Jonathan Beard
 * @version: Sat March  20 09:04:38 2021
 * 
 * Copyright 2014 Jonathan Beard
 * Copyright 2021 Jonathan Beard
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
#ifndef RAFTPORTMAP_T_HPP
#define RAFTPORTMAP_T_HPP  1

#include <map>
#include <unordered_map>

#include <string>
#include <mutex>
#include "port_info.hpp"
#include "defs.hpp"

struct portmap_t
{
   portmap_t() = default;
   virtual ~portmap_t() = default;


#ifdef STRING_NAMES    
   std::map< raft::port_key_type, PortInfo > map;
#else
   /**
    * integer lookup for port name to port info object,
    * which is all the data describing the port itself. 
    */
   std::unordered_map< raft::port_key_type, PortInfo > map;
   /**
    * for each port, keep a map of the "name" to string
    * name representation. Otherwise once we have integer
    * keys enabled you won't easily be able to debug...
    */
   std::unordered_map< raft::port_key_type, 
                       raft::port_key_name_t > name_map;
#endif
   /**
    * mutex used to add/subtract from port. Not used in 
    * most circumstances, only once the application gets
    * executing do we really need this. 
    */
   std::mutex                        mutex_map;
};

#endif /* END RAFTPORTMAP_T_HPP */
