/**
 * filter.tcc - A one dimensional filter library, eventually it'll
 * be expanded to cover n-dimensions but for now this is what I needed
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
#include <vector>
#include <array>
#include <cmath>
#include <exception>

enum FilterType { Gaussian, LaplacianGaussian };


template < typename D, 
           std::uint16_t RADIUS > class filterbase
{
public:
   filterbase()
   {

   }
   virtual ~filterbase()
   {
   
   }

   /**
    * apply - apply the filter to data in container c, output
    * the information to the vector output.  The filter will not
    * pad the first and last elements so the output values will 
    * naturally start at index RADIUS with respect to the input
    * array c.
    * @param   c - type C
    * @param   output - std::vector< D >
    */
   template< class C > 
      void apply( C c, std::vector< D > &output )
   {
      /** assume c is a container **/
      if( c.size() < ( RADIUS * 2 ) + 1 )
      {
         /** TODO, replace with raft::exception once you write it **/
         std::cerr << "size of 'c' must be at least the size of the filter.\n";
         exit( 0 );
      }
      /** else apply filter **/
      for( auto it_center( c.begin() + RADIUS ); 
            it_center != ( c.end() - RADIUS ); ++it_center )
      {
         std::int32_t index( 0 );
         D value( (D) 0 );
         for( auto it( it_center - RADIUS ); 
               it != (it_center + (RADIUS * 2 + 1) ); ++it, index++ )
         {
               value += ((*it) * arr[ index ]);
         }
         output.push_back( value );
      }
   }
   
   void standardize()
   {
      D total( (D) 0 );
      for( const auto val : arr )
      {
         total += arr;
      }
      for( auto &val : arr )
      {
         val = val - arr;
      }
   }



protected:


   /** stores the actual filter **/
   std::array< D, (RADIUS * 2) + 1 > arr;
};


template < typename D, 
           std::uint16_t RADIUS, 
           FilterType TYPE > class filter;


/** filter with standard deviation of 1/2, TODO, make computable **/
template < typename D, 
           std::uint16_t RADIUS > class filter< D, 
                                                RADIUS, 
                                                Gaussian > : public filterbase< D, 
                                                                                RADIUS >
{
public:
   filter() : filterbase< D, RADIUS >()
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
      std::for_each( (this)->arr.begin(), (this)->arr.end(), filter_func );
   }
   
   virtual ~filter()
   {

   }
};

template < typename D, 
           std::uint16_t RADIUS > class filter< D, 
                                                RADIUS, 
                                                LaplacianGaussian > : 
                                                   public filterbase< D, 
                                                                      RADIUS >
{
public:
   filter() : filterbase< D, RADIUS >()
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
      std::for_each( (this)->arr.begin(), (this)->arr.end(), filter_func );
   }
   
   virtual ~filter()
   {

   }
};
#endif /* END _FILTER_TCC_ */
