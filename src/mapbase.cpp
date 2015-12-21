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

MapBase::MapBase()
{
  
}

MapBase::~MapBase()
{
   auto &container( all_kernels.acquire() );
   for( raft::kernel *kern : container )
   {
      if( kern != nullptr ) 
      {   
         delete( kern );
      }
   }
   all_kernels.release();
}


void
MapBase::join( raft::kernel &a, const std::string name_a, PortInfo &a_info, 
               raft::kernel &b, const std::string name_b, PortInfo &b_info )
{
   if( a_info.type != b_info.type )
   {
      std::stringstream ss;
      ss << "When attempting to join ports (" << common::printClassName( a ) <<  
         "[" << name_a << "] -> " << common::printClassName( b ) << "[" << 
         name_b << "] have conflicting types.  " << 
            common::printClassNameFromStr( a_info.type.name() ) <<  
         " and " << common::printClassNameFromStr( b_info.type.name() ) << "\n"; 
      throw PortTypeMismatchException( ss.str() );
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
   PortInfo &i_in( i->input.getPortInfoFor( "0" ) ),
            &i_out( i->output.getPortInfoFor( "0" ) );
   join( *a, a_out.my_name, a_out,
         *i, i_in.my_name, i_in );
   join( *i, i_out.my_name, i_out,
         *b, b_in.my_name, b_in );
}
