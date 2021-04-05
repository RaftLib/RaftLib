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
#include <utility>
#include "port_info.hpp"
#include "portmap_t.hpp"
#include "portiterator.hpp"

PortIterator::PortIterator( portmap_t * const port_map )
	: map_iterator(port_map->map.begin())
{

}

PortIterator::PortIterator( portmap_t * const port_map, 
                            const std::size_t index )
	: map_iterator(index == port_map->map.size() 
        ? port_map->map.end()
        : std::next(port_map->map.begin(), index) )
{
}

PortIterator&
PortIterator::operator++() 
{
  ++map_iterator;
   return( (*this) );
}

const raft::port_key_type&
PortIterator::name() const
{
    return( map_iterator->first );
}

bool
PortIterator::operator==( const PortIterator &rhs ) const
{
   return( map_iterator == rhs.map_iterator );
}

bool 
PortIterator::operator!=( const PortIterator &rhs ) const
{
   return( map_iterator != rhs.map_iterator );
}

FIFO&
PortIterator::operator*() const
{ 
   return(*map_iterator->second.getFIFO());
}
