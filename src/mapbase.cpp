/**
 * mapbase.cpp - 
 * @author: Jonathan Beard
 * @version: Fri Sep 12 10:28:33 2014
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
#include <sstream>
#include <map>
#include <sstream>
#include <vector>
#include <typeinfo>
#include <cstdio>
#include <cassert>
#include <string>
#include <cstdlib>
#include <cstring>

#include "common.hpp"
#include "mapbase.hpp"
#include "graphtools.hpp"
#include "demangle.hpp"

MapBase::MapBase()
{
  
}

MapBase::~MapBase()
{
    auto &the_container( internally_created_kernels.acquire() );
    for( auto *kernel : the_container )
    {
        /**
         * NOTE: if we re-add the dynamic add code that doesn't
         * lock the list on remove, and sets this list entry to 
         * nullptr after calling detructor, then we might need to
         * re-add the nullptr check, or make this a custom 
         * asynch structure vs. the stdlib container it is now. 
         */
        delete( kernel );
    }
    /** 
     * as long as this derives from something that meets the "sequence container" def
     * we should be good to use clear.
     */
    the_container.clear();
    internally_created_kernels.release();
}


void
MapBase::join( raft::kernel &a, const raft::port_key_type name_a, PortInfo &a_info, 
               raft::kernel &b, const raft::port_key_type name_b, PortInfo &b_info )
{
   if( a_info.type != b_info.type )
   {
      std::stringstream ss;
      ss << "Error found when attempting to join kernel \"" << 
        common::printClassName( a ) <<  "\" with port " << 
         "[" << a.output.getPortName( name_a ) << "] to " << " kernel \"" << 
            common::printClassName( b ) << "\" with port [" << 
         b.input.getPortName( name_b ) << "], their types must match. " << 
         " currently their types are (" << 
            common::printClassNameFromStr( a_info.type.name() ) <<  
         " -and- " << common::printClassNameFromStr( b_info.type.name() ) << ")."; 
      throw PortTypeMismatchException( ss.str() );
   }
   if( a_info.other_kernel != nullptr )
   {
    //FIXME
      throw PortDoubleInitializeException( "port double initialized with: " ); //+ std::to_string( name_b ) );
   }
   if( b_info.other_kernel != nullptr )
   {
      //FIXME
      throw PortDoubleInitializeException( "port double initialized with: " ); //+ std::to_string( name_a ) );
   }
   a_info.other_kernel = &b;
   a_info.other_name   = name_b;
   b_info.other_kernel = &a;
   b_info.other_name   = name_a;
}
   
void 
MapBase::insert( raft::kernel *a,  PortInfo &a_out, 
                 raft::kernel *b,  PortInfo &b_in,
                 raft::kernel *i)
{
#ifdef STRING_NAMES
   PortInfo &i_in( i->input.getPortInfoFor( "0" ) ),
            &i_out( i->output.getPortInfoFor( "0" ) );
#else
   PortInfo &i_in( i->input.getPortInfoFor( raft::port_key_type( 0 ) ) ),
            &i_out( i->output.getPortInfoFor( raft::port_key_type( 0 ) ) );
#endif
   join( *a, a_out.my_name, a_out,
         *i, i_in.my_name, i_in );
   join( *i, i_out.my_name, i_out,
         *b, b_in.my_name, b_in );
}
   
void MapBase::portNotFound( const bool src, 
                            const AmbiguousPortAssignmentException &ex, 
                            raft::kernel * const k )
{
      std::stringstream ss;
      const auto name( raft::demangle( typeid( *k ).name() ) );
      if( src )
      {
         ss << ex.what() << "\n";
         ss << "Output port from source kernel (" << name << ") " <<
                "has more than a single port.";

      }
      else
      {
         ss << ex.what() << "\n";
         ss << "Input port from destination kernel (" << name << ") " <<
                "has more than a single port.";
      }
      throw AmbiguousPortAssignmentException( ss.str() );
}
