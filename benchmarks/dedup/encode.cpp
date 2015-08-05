/**
 * encode.cpp - 
 * @author: Jonathan Beard
 * @version: Wed Aug  5 12:37:30 2015
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
#include "encode.hpp"
#include <cstdint>
#include <cstring>

std::uint32_t
encode::hash_from_key_fn( void * const k )
{
   //NOTE: sha1 sum is integer-aligned
   return( reinterpret_cast< std::uint32_t* >( k )[ 0 ] );
}

bool
encode::keys_equal_fn( void * const key1, void * const key2 )
{
  return( std::memcmp( key1, key2, SHA1_LEN ) == 0 ); 
}
