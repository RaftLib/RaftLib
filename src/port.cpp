/**
 * port.cpp - 
 * @author: Jonathan Beard
 * @version: Thu Aug 28 09:55:47 2014
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
#include <cassert>
#include <typeinfo>
#include <typeindex>
#include <sstream>
#include <iostream>

#include "fifo.hpp"
#include "kernel.hpp"
#include "port.hpp"
#include "portexception.hpp"

Port::Port( raft::kernel * const k ) : PortBase(),
                                       kernel( k )
{
}

Port::Port( raft::kernel *k, 
            void * const ptr, 
            const std::size_t nbytes ) :
   PortBase(),
   kernel( k ),
   alloc_ptr( ptr ),
   alloc_ptr_length( nbytes )
{
}


const std::type_index&
Port::getPortType( const std::string &&port_name )
{
   const auto ret_val( portmap.map.find( port_name ) );
   if( ret_val == portmap.map.cend() )
   {
      throw PortNotFoundException( "Port not found for name \"" + port_name + "\"" );
   }
   return( (*ret_val).second.type );
}

FIFO&
Port::operator[]( const std::string &&port_name )
{
   //NOTE: We'll need to add a lock here if later
   //we intend to remove ports dynamically as well
   //for the moment however lets just assume we're
   //only adding them
   const auto ret_val( portmap.map.find( port_name ) );
   if( ret_val == portmap.map.cend() )
   {
      throw PortNotFoundException( 
         "Port not found for name \"" + port_name + "\"" );
   }
   return( *((*ret_val).second.getFIFO())  );
}

bool
Port::hasPorts()
{
   return( portmap.map.size() > 0 ? true : false );
}

PortIterator
Port::begin()
{
   return( PortIterator( &portmap ) );
}

PortIterator
Port::end()
{
   return( PortIterator( &portmap, portmap.map.size() ) );
}

std::size_t
Port::count()
{
   return( (std::size_t) portmap.map.size() );
}

PortInfo&
Port::getPortInfoFor( const std::string port_name )
{
   const auto ret_val( portmap.map.find( port_name ) );
   if( ret_val == portmap.map.cend() )
   {
      std::stringstream ss;
      ss << "Port not found for name \"" << port_name << "\"";
      throw PortNotFoundException( ss.str() );
   }
   return( (*ret_val).second );
}

PortInfo&
Port::getPortInfo()
{
   if( portmap.map.size() > 1 )
   {
      /** TODO: extract kernel name to go here too **/
      throw PortNotFoundException( "One port expected, more than one found!" );
   }
   auto pair( portmap.map.begin() );
   return( (*pair).second );
}
