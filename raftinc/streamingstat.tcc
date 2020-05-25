/**
 * streamingstat.tcc - class for various streaming statistics, 
 * eventually we'll expand it to include more higher order
 * moments.
 *
 * @author: Jonathan Beard
 * @version: Mon Jan 12 09:11:31 2015
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
#ifndef RAFTSTREAMINGSTAT_TCC
#define RAFTSTREAMINGSTAT_TCC  1

#include <cstdint>
#include <cmath>
#include <cstddef>
#include <type_traits>

namespace raft
{

template < typename T > class streamingstat
{
public:
   streamingstat() = default;

   virtual ~streamingstat() = default;

   /**
    * mean - returns the mean of the population seen
    * as a F type (currently a double)
    * @return F
    */
   template <  typename F,
               typename std::enable_if< 
                        std::is_floating_point< F >::value >::type* = nullptr > 
   F mean()
   {
      return( static_cast< F >( sum ) / static_cast< F >( N ) );
   }

   /** 
   * std - returns estimate of the standard deviation for
   * the population seen so far.
   * @return F - streaming mean
   */
   template <  typename F,
               typename std::enable_if< 
                        std::is_floating_point< F >::value >::type* = nullptr > 
   F std()
   {
      const F a(  sum_square / (F) N );
      const F b( (F) sum / (F) N );
      const F b_squared( b * b );
      return( std::sqrt( a - b_squared ) );
   }
   
   
   /**
    * update - updates the streaming statistics within this
    * object with the value in the parameter.
    * @param value - const T, value to update with
    */
   void update( const T value )
   {
      sum        += value;
      /** 
       * if we were to get lots of power of two's for value, then
       * adding a check for them might shave a new nanoseconds per
       * op...but not worth it unless we can ensure that.
       */
      sum_square += (value * value);
      N++;
   }
   

   /**
    * reset - does the obious, resets the stats saved
    * in this object all to zero.
    */
   void reset()
   {
      sum        = static_cast< T >( 0 );
      sum_square = static_cast< T >( 0 );
      N          = 0;
   }

private:
   T               sum        = static_cast< T >( 0 );
   T               sum_square = static_cast< T >( 0 );
   std::uintmax_t  N          = 0;
};

} /** end namespace raft **/
#endif /* END RAFTSTREAMINGSTAT_TCC */
