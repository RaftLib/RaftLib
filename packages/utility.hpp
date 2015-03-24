/**
 * utility.hpp - 
 * @author: Jonathan Beard
 * @version: Sat Mar 21 22:12:15 2015
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
#ifndef _UTILITY_HPP_
#define _UTILITY_HPP_  1
#include <vector>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <cassert>

namespace raft
{

template < typename A,
           typename B> 
static
std::vector< typename std::common_type< A, B >::type > 
irange( const A a, 
        const B b, 
        const std::size_t delta = 1 )
{
  static_assert( std::is_integral< A >::value, "A must be an integral type" );
  static_assert( std::is_integral< B >::value, "B must be an integral type" );
  using common_t = 
   typename std::common_type< A, B >::type;
  if( a < b )
  {
     const auto cap( (b - a + 1) / delta );
     std::vector< common_t > out( cap );
     auto index( 0 );
     for( auto i( a ); i <= b; i += delta, index++ )
     {
        out[ index ] = i;
     }
     return( std::forward< std::vector< common_t > >( out ) );
  }
  else
  {
     const auto cap( (a - b + 1) / delta );
     std::vector< common_t > out( cap );
     auto index( 0 );
     for( auto i( a ); i >= b; i -= delta, index++ )
     {
        out[ index ] = i;
     }
     return( std::forward< std::vector< common_t > >( out ) );
  }
}

}
#endif /* END _UTILITY_HPP_ */
