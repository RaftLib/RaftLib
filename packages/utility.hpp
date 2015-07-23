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
#include <utility>
#include <type_traits>

namespace raft
{

template < typename A, 
           typename B > using common_t = typename std::common_type< A, B >::type;

template < typename A, 
           typename B > using common_v_t = std::vector< common_t< A, B > >;

/** 
 * irange - simple utility that returns a vector of 
 * integers with the common type of A & B, useful for
 * some instances when a range is needed....however
 * don't be too tempted to use this over and over 
 * in a tight loop since the compiler will more than
 * likely create lots of copies.
 * @param   a - const A, start of range (inclusive)
 * @param   b - const B, end of range   (inclusive)
 * @param   delta - const std::size_t 
 * @return std::vector< T >, where T is common type of 
 *         A & B
 */
template < typename A,
           typename B > 
static
common_v_t< A, B >
irange( const A a, 
        const B b, 
        const std::size_t delta = 1 )
{
  static_assert( std::is_integral< A >::value, "A must be an integral type" );
  static_assert( std::is_integral< B >::value, "B must be an integral type" );
  if( a < b )
  {
     const auto cap( (b - a + 1) / delta );
     common_v_t< A, B > out( cap );
     for( auto i( a ), index( 0 ); i <= b; i += delta, index++ )
     {
        out[ index ] = i;
     }
     return( std::forward< common_v_t< A, B > >( out ) );
  }
  else
  {
     const auto cap( (a - b + 1) / delta );
     common_v_t< A, B > out( cap );
     for( auto i( a ), index( 0 ); i >= b; i -= delta, index++ )
     {
        out[ index ] = i;
     }
     return( std::forward< common_v_t< A, B > >( out ) );
  }
}

/** 
 * FIXME, this can be a whole lot cooler, add struct recursion with varargs and vectorize 
 * but I want to get it working, so I'm settling for the faster to type but less cool
 * version
 */
template < class T, class F, typename std::enable_if< std::is_arithmetic< T >::value >::type* = nullptr > 
static 
void
sum( F &&dst, F &&a, F &&b )
{
   auto &dst_ref( dst.template allocate< T >() );
   dst_ref = a.template peek< T >() + b.template peek< T >();
   a.recycle();
   b.recycle();
   dst.send();
}

}
#endif /* END _UTILITY_HPP_ */
