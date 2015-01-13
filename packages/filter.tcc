/**
 * filter.tcc - 
 * @author: Jonathan Beard
 * @version: Mon Jan 12 10:12:30 2015
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
#ifndef _FILTER_TCC_
#define _FILTER_TCC_  1
#include <algorithm>
#include <cstdint>
#include <cstdlib>

enum FilterType { Gaussian, LaplacianGaussianFilter };


template < typename D, 
           std::uint16_t RADIUS > filterbase
{
public:
   filterbase() = default;
   virtual ~filterbase() = default;

   template< class C > 
      void apply( C c, std::vector< D > &output )
   {
      /** assume c is a container **/
      if( c.size() < ( RADIUS * 2 ) + 1 )
      {
         throw Exception( "sizes don't match!\n" );
      }
      /** else apply filter **/
   }

protected:
   std::array< D, (RADIUS * 2) + 1 > arr;
};


template < typename D, 
           std::uint16_t RADIUS, 
           FilterType TYPE > filter;

template < typename D, std::uint16_t RADIUS > filter< D, 
                                                      RADIUS, 
                                                      Gaussian >
{
public:
   filter()
   {
      /** filter function **/
      std::int32_t x( - RADIUS );
      auto filter_func( [&]( D &val ) -> void
      {
         const double half_x_squared( (double)(-(x*x)) / 2.0 );
         const double numerator( std::exp( half_x_squared ) );
         const double denominator( 2.506628274631000502415765 );
         val = ( numerator / denominator );
         x++;
      } );
      /** encode filter **/
      std::for_each( arr.begin(), arr.end(), filter_func );
   }
   
   virtual ~filter()
   {

   }

private:
   D arr[ (2 * RADIUS) + 1 ];
};
#endif /* END _FILTER_TCC_ */
