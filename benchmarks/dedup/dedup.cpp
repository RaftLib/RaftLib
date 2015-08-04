/**
 * dedup.cpp - 
 * @author: Jonathan Beard
 * @version: Thu Jul 30 20:33:57 2015
 * 
 * Copyright 2015 Jonathan Beard
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

#include <cstdint>
#include <iostream>
#include <cstddef>
#include <cstdlib>

#include "murmur.hpp"

int
main( int argc, char **argv )
{
   const char * str( "this is a great hash function" );
   const char * str2( "required by applicable law" );
   const char * str3( "this is a great hash function" );
   const auto str_one( murmur::murmur3_32( str, strlen( str ), 0x255 ) );
   const auto str_two( murmur::murmur3_32( str2, strlen( str2 ), 0x255 ) );
   const auto str_three( murmur::murmur3_32( str3, strlen( str3 ), 0x255 ) );
   std::cout << str_one << " - " << str_two << " - " << str_three << "\n";
   return( EXIT_SUCCESS );
}
