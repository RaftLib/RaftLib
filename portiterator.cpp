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
#include <pthread.h>
#include "port_info.hpp"
#include "portmap_t.hpp"
#include "portiterator.hpp"
#include "pthreadwrap.h"
PortIterator::PortIterator( portmap_t * const port_map ) : port_map( port_map )
{
   PortIterator::initKeyMap( port_map, key_map );
   pthread_mutex_lock_d( &(port_map->mutex_map), __FILE__, __LINE__ ); 
}

PortIterator::PortIterator( portmap_t * const port_map, 
                            const std::size_t index ) : 
                                                    port_map( port_map ),
                                                    map_index( index )
{
   is_end = true;
   PortIterator::initKeyMap( port_map, key_map );
}

PortIterator::PortIterator( const PortIterator &it ) : port_map( it.port_map ),
                                                       map_index( it.map_index )
{
   PortIterator::initKeyMap( port_map, key_map );
}

PortIterator::~PortIterator()
{
   if( is_end )
   {
      pthread_mutex_unlock( &(port_map->mutex_map) );
   }
}

PortIterator&
PortIterator::operator++() noexcept
{
   map_index++;
   return( (*this) );
}

bool
PortIterator::operator==( const PortIterator &rhs ) noexcept
{
   /** 
    * TODO, on a more philosophical note, should this
    * be a ptr comparison for the FIFO's but then the 
    * end function would be harder to implement
    */
   return( map_index == rhs.map_index );
}

bool 
PortIterator::operator!=( const PortIterator &rhs ) noexcept
{
   return( map_index != rhs.map_index );
}

FIFO&
PortIterator::operator*() noexcept
{ 
   return(
      (*port_map->map[ key_map[ map_index ] ].getFIFO() ) );
}

void
PortIterator::initKeyMap( portmap_t * const port_map, 
                          std::vector< std::string > &key_map ) noexcept
{
   std::map< std::string, PortInfo > &map_ref( port_map->map );
   for( const auto &pair : map_ref )
   {
      key_map.emplace_back( pair.first );
   }
}
