/**
 * tempmap.hpp - Defines an interface to create temp-mappings
 * which are basically maps that are allowed to have unconnected
 * inputs and/or outputs that will be connected within a main
 * mapping. The only real rule to these "temp-maps" is that the 
 * names of the output ports must be unique.
 *
 * @author: Jonathan Beard
 * @version: Sun Nov 30 06:12:23 2014
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
#ifndef _TEMPMAP_HPP_
#define _TEMPMAP_HPP_  1
#include <map>
#include <string>

#include "mapbase.hpp"

namespace raft
{
    class map;

class temp_map : public MapBase
{
public:
    temp_map();
    virtual ~temp_map();


protected:
   friend class map;
   /** all needed data structures in mapbase **/
};

} /** end namespace raft **/
#endif /* END _TEMPMAP_HPP_ */
