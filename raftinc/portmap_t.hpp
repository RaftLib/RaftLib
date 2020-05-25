/**
 * portmap_t.hpp - 
 * @author: Jonathan Beard
 * @version: Sun Oct  5 09:04:38 2014
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
#ifndef RAFTPORTMAP_T_HPP
#define RAFTPORTMAP_T_HPP  1

#include <map>
#include <string>
#include <mutex>
#include "port_info.hpp"

struct portmap_t
{
   portmap_t() = default;
   virtual ~portmap_t() = default;

   std::map< std::string, PortInfo > map;
   std::mutex                        mutex_map;
};

#endif /* END RAFTPORTMAP_T_HPP */
