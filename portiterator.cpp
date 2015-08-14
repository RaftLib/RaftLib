/**
 * portiterator.cpp - 
 * @author: Jonathan Beard
 * @version: Sun Oct  5 08:49:11 2014
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
#include <algorithm>
#include <utility>
#include <map>
#include <iostream>

#include "port_info.hpp"
#include "portmap_t.hpp"
#include "portiterator.hpp"

PortIterator::PortIterator( portmap_t *port_map ) : port_map( port_map )
{
   PortIterator::initKeyMap( port_map, key_map );
   /** TODO, might should be trylock with error if already lock, same for below **/
   //port_map->map_mutex.lock();
}

PortIterator::PortIterator( portmap_t *port_map, 
                            const std::size_t index ) : 
                                                    port_map( port_map ),
                                                    map_index( index )
{
   PortIterator::initKeyMap( port_map, key_map );
}

PortIterator::PortIterator( const PortIterator &it ) : port_map( it.port_map ),
                                                       map_index( it.map_index )
{
   PortIterator::initKeyMap( port_map, key_map );
}

PortIterator::~PortIterator()
{
   //port_map->map_mutex.unlock();
}

PortIterator&
PortIterator::operator++()
{
   //TODO, perhaps throw error if out of bounds here
   map_index++;
   return( (*this) );
}

bool
PortIterator::operator==( const PortIterator &rhs )
{
   /** 
    * TODO, on a more philosophical note, should this
    * be a ptr comparison for the FIFO's but then the 
    * end function would be harder to implement
    */
   return( map_index == rhs.map_index );
}

bool 
PortIterator::operator!=( const PortIterator &rhs )
{
   return( map_index != rhs.map_index );
}

FIFO&
PortIterator::operator*()
{ 
   return(
      (*port_map->map[ key_map[ map_index ] ].getFIFO() ) );
}

void
PortIterator::initKeyMap( portmap_t *port_map, std::vector< std::string > &key_map )
{
   std::map< std::string, PortInfo > &map_ref( port_map->map );
   for( const auto &pair : map_ref )
   {
      key_map.push_back( pair.first );
   }
}
