/**
 * port.cpp - 
 * @author: Jonathan Beard
 * @version: Sun July 23 06:22 2017
 * 
 * Copyright 2017 Jonathan Beard
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
#include "demangle.hpp"

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
Port::getPortType( const raft::port_key_type &&port_name )
{
   const auto ret_val( portmap.map.find( port_name ) );
   if( ret_val == portmap.map.cend() )
   {
      const auto actual_port_name = getPortName( port_name );
      throw PortNotFoundException( "Port not found for name \"" + actual_port_name + "\"" );
   }
   return( (*ret_val).second.type );
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
    
std::string 
Port::getPortName( const raft::port_key_type n )
{
#ifdef STRING_NAMES
    return( n );
#else
    const auto ret_val( portmap.name_map.find( n ) );
    if( ret_val == portmap.name_map.cend() )
    {
       std::stringstream ss;
       ss << "Port not found for name \"" << n << "\"";
       throw PortNotFoundException( ss.str() );
    }
    return( (*ret_val).second );
#endif   
}

PortInfo&
Port::getPortInfoFor( const raft::port_key_type port_name )
{
   const auto ret_val( portmap.map.find( port_name ) );
   if( ret_val == portmap.map.cend() )
   {
      std::stringstream ss;
      ss << "Port not found for name \"" << getPortName( port_name ) << "\"";
      throw PortNotFoundException( ss.str() );
   }
   return( (*ret_val).second );
}

PortInfo&
Port::getPortInfo()
{
   const auto number_of_ports( portmap.map.size() );
   if( number_of_ports > 1 )
   {
      /** 
       * NOTE: This is cought and re-thrown within the 
       * runtime within mapbase.hpp so that we can push
       * out the name of the kernel and a bit more info
       */
      throw AmbiguousPortAssignmentException( "One port expected, more than one found!" );
   }
   else if( number_of_ports == 0 )
   {
      const auto name( raft::demangle( typeid( (*this->kernel) ).name() ) );
      throw PortNotFoundException( "At least one port must be defined, none were for kernel class \"" + name + "\"" );
   }
   auto pair( portmap.map.begin() );
   return( (*pair).second );
}

#ifdef STRING_NAMES
FIFO& Port::operator[]( const raft::port_key_type  &&port_name  )
{
    //NOTE: We'll need to add a lock here if later
    //we intend to remove ports dynamically as well
    //for the moment however lets just assume we're
    //only adding them
    const auto ret_val( portmap.map.find( port_name ) );
    if( ret_val == portmap.map.cend() )
    {
       throw PortNotFoundException( 
          "Port not found for name \"" + getPortName( port_name ) + "\"" );
    }
    return( *((*ret_val).second.getFIFO())  );
}

FIFO& Port::operator[]( const raft::port_key_type  &port_name )
{
    //NOTE: We'll need to add a lock here if later
    //we intend to remove ports dynamically as well
    //for the moment however lets just assume we're
    //only adding them
    const auto ret_val( portmap.map.find( port_name ) );
    if( ret_val == portmap.map.cend() )
    {
       throw PortNotFoundException( 
          "Port not found for name \"" + getPortName( port_name ) + "\"" );
    }
    return( *((*ret_val).second.getFIFO())  );
}
#endif
