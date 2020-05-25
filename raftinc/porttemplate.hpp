/**
 * porttemplate.hpp - This object is designed to be used in 
 * conjunction with a SubMap object, essentially its just 
 * a container to hold reference to the input and output
 * ports to this sub-map so that the library doesn't have 
 * to search the entire map when the SubMap object is 
 * added to the main map.
 *
 * @author: Jonathan Beard
 * @version: Sun Nov 30 10:09:25 2014
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
#ifndef RAFTPORTTEMPLATE_HPP
#define RAFTPORTTEMPLATE_HPP  1
#include <map>
#include "port_info.hpp"

class PortTemplate
{
public:
   PortTemplate();

   virtual ~PortTemplate();

protected:  
   std::map< std::string, PortInfo& > map; 
};
#endif /* END RAFTPORTTEMPLATE_HPP */
