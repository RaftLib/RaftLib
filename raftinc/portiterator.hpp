/**
 * portiterator.hpp -
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
#ifndef RAFTPORTITERATOR_HPP
#define RAFTPORTITERATOR_HPP  1
#include <iterator>
#include <map>
#include <cstddef>

#include "portmap_t.hpp"

class FIFO;

class PortIterator : public std::iterator< std::forward_iterator_tag, FIFO >
{
public:
    explicit PortIterator( portmap_t * port_map ) :
        map_iterator( port_map->map.begin() )
    {
    }

    PortIterator( portmap_t * port_map, std::size_t index ) :
        map_iterator( index == port_map->map.size() ?
                               port_map->map.end() :
                               std::next( port_map->map.begin(), index ) )
    {
    }

    PortIterator& operator++()
    {
        ++map_iterator;
        return( ( *this ) );
    }

    bool operator==( const PortIterator& rhs ) const
    {
        return( map_iterator == rhs.map_iterator );
    }
    bool operator!=( const PortIterator& rhs ) const
    {
        return( map_iterator != rhs.map_iterator );
    }
    FIFO& operator*() const
    {
        return( *map_iterator->second.getFIFO() );
    }

    const raft::port_key_type& name() const
    {
        return( map_iterator->first );
    }

private:
    using map_iterator_type = std::decay_t<decltype(begin(portmap_t::map))>;

    map_iterator_type map_iterator;
};

#endif /* END RAFTPORTITERATOR_HPP */
