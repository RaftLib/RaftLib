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
#include <cmath>
#include <map>
#include <memory>


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
           typename B,
           typename std::enable_if< std::is_arithmetic< A >::value >::type* = nullptr,
           typename std::enable_if< std::is_arithmetic< B >::value >::type* = nullptr > 
static
constexpr
common_v_t< A, B >
range( const A a, 
       const B b, 
       const common_t< A, B > delta = 1 )
{
  if( std::isless( a, b ) )
  {
     const auto cap( (b - a + delta) / delta );
     common_v_t< A, B > out( cap );
     auto index( 0 );
     for( common_t< A, B > i( a ); 
      std::islessequal( i, b + delta ); i += delta, index++ )
     {
        out[ index ] = i;
     }
     return( std::move( out ) );
  }
  else
  {
     const auto cap( (a - b + delta ) / delta );
     common_v_t< A, B > out( cap );
     auto index( 0 );
     for( common_t< A, B > i( a ); 
      std::isgreaterequal( i, b - delta ); i -= delta, index++ )
     {
        out[ index ] = i;
     }
     return( std::move( out ) );
  }
}


/**
 * intersect_map - Take in two maps of the same key type,
 * and return a std::unique_ptr< map > with the intersection
 * of the two maps. The iterators to these maps are sorted
 * so that we can make this a linear operation with the
 * length of N. The value can be different, keys must be 
 * the same type.
 *
 * @param   a  - const std::map<KeyType, LeftValue>
 * @param   b - const std::map<KeyType, RightValue>
 * @return  std::unique_ptr< std::map<KeyType, std::pair< LeftValue, RightValue> > >
 */
template< typename KeyType, 
          typename LeftValue, 
          typename RightValue >
static 
std::unique_ptr< std::map<KeyType, std::pair< LeftValue, RightValue> > >
intersect_map( const std::map<KeyType, LeftValue>  &a, 
               const std::map<KeyType, RightValue> &b )
{
    using map_t = std::map< KeyType, std::pair< LeftValue, RightValue > >;
    std::unique_ptr< map_t > result( new map_t() );

    auto it_a( a.cbegin() );
    auto it_b( b.begin() );

    while( it_a != a.cend() && it_b != b.cend() )
    {
        if( it_a->first < it_b->first )
        {
            ++it_a;
        }
        else if (it_b->first < it_a->first)
        {
            ++it_b;
        }
        else
        {
            result->insert( std::make_pair( it_a->first, 
                                            std::make_pair( it_a->second, 
                                                            it_b->second )
                                           )
                          );
            ++it_a;
            ++it_b;
        }
    }
    return( result );
}



} /** end namespace raft **/
#endif /* END _UTILITY_HPP_ */
