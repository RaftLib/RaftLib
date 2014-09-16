/**
 * map.cpp - 
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
#include "map.hpp"

Map::Map()
{
  
}

std::set< Kernel* >&
Map::get_source_kernels()
{
   return( source_kernels );
}

std::set< Kernel* >&
Map::get_all_kernels()
{
   return( all_kernels );
}

std::set< edge_t >&
{
   return( all_edges );
}


void
Map::join( Kernel &a, const std::string name_a, PortInfo &a_info, 
           Kernel &b, const std::string name_b, PortInfo &b_info )
{
   if( a_info.type != b_info.type )
   {
      int status;
      throw PortTypeMismatchException( "When attempting to join ports (" + 
         abi::__cxa_demangle( typeid( a ).name(), 0, 0, &status ) + "[" + 
         name_a + "] -> " + 
         abi::__cxa_demangle( typeid( b ).name(), 0, 0, &status ) + "[" + 
         name_b + "] have conflicting types.  " + 
         abi::__cxa_demangle( a_info.type.name(), 0, 0, &status ) + 
         " and " + abi::__cxa_demangle( b_info.type.name(), 0, 0, &status ) + "\n" );
   }
   a_info.other_kernel = &b;
   a_info.other_name   = name_b;
   b_info.other_kernel = &a;
   b_info.other_name   = name_a;
}
