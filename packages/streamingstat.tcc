/**
 * streamingstat.tcc - 
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

   
   double  mean()
   {
      return( (double) T / (double) N );
   }

   double std()
   {
      const double a( (double) sum_square / (double) N );
      const double b( (double) sum / (double) N )
      const double b_squared( b * b );
      return( std::sqrt( a - b_squared ) );
   }
   
   void update( const T value )
   {
      sum        += value;
      sum_square += (value * value);
      N          += 1;
   }

private:
   T  sum            = ( T ) 0;
   T  sum_square     = ( T ) 0;
   std::size_t N     = 0;
};

} /** end namespace raft **/
#endif /* END _STREAMINGSTAT_TCC_ */
