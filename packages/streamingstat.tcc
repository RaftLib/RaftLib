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
#ifndef _STREAMINGSTAT_TCC_
#define _STREAMINGSTAT_TCC_  1

#include <cstdint>
#include <cmath>
#include <cstddef>

namespace raft
{
using statfloat_t = double;

template < typename T > class streamingstat
{
public:
   streamingstat()
   {
      /** also nothing to do yet **/
   }

   virtual ~streamingstat()
   {
      /** nothing to do yet **/
   }

   /**
    * mean - returns the mean of the population seen
    * as a statfloat_t type (currently a double)
    * @return statfloat_t
    */
   statfloat_t mean()
   {
      return( (double) sum / (double) N );
   }

   /** 
   * std - returns estimate of the standard deviation for
   * the population seen so far.
   * @return statfloat_t - streaming mean
   */
   statfloat_t std()
   {
      const statfloat_t a( (statfloat_t) sum_square / (statfloat_t) N );
      const statfloat_t b( (statfloat_t) sum / (statfloat_t) N );
      const statfloat_t b_squared( b * b );
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
      sum_square += (value * value);
      N          += 1;
   }

   /**
    * reset - does the obious, resets the stats saved
    * in this object all to zero.
    */
   void reset()
   {
      sum        = (T) 0;
      sum_square = (T) 0;
      N          = 0;
   }

private:
   T  sum            = ( T ) 0;
   T  sum_square     = ( T ) 0;
   std::size_t N     = 0;
};

} /** end namespace raft **/
#endif /* END _STREAMINGSTAT_TCC_ */
