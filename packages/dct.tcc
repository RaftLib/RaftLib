/**
 * dct.tcc - raftlib kernel for discrete cosine transform, with specialized versions for 
 * the 8x8 cosine transform.
 * @author: Jonathan Beard
 * @version: Thu Jul 23 18:37:46 2015
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
#ifndef _DCT_TCC_
#define _DCT_TCC_  1
#include <raft>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <type_traits>
#include <functional>
#include <iostream>
#include <algorithm>
#include <iomanip>
namespace raft
{

enum DCTTYPE : std::int32_t { NDIM, x88 };

template < typename T,
           std::size_t N,
           class Enable = void > struct matrix{};

/** generic square matrix **/
template < typename T, 
           std::size_t N >  struct matrix< T, N, 
           typename std::enable_if< std::is_fundamental< T >::value >::type  >
{
   T              arr[ N * N ];;
   decltype( N )  dim = N;

   /** 
    * [] - for convenience, references arr inside
    * square matrix.
    * @return T&
    */
   T& operator []( const std::size_t index )
   {
      return(  arr[ index ] );
   }

   /**
    * << - for pretty printing the matrix
    * @param   stream - std::ostream&, output stream
    * @param   m - matrix< T, N >, input matrix
    * @return  std::ostream&, same stream from input for chaining
    */
   friend std::ostream& operator << ( std::ostream &stream, matrix< T, N > &m )
   {
      for( auto x( 0 ); std::isless( x,  8 ); x++ )
      {
         for( auto y( 0 ); std::isless( y, 8 ); y++ )
         {
            stream << std::fixed <<  std::setw( 7 ) << std::setprecision( 2 );
            stream << m[ ( x * 8 ) + y ];
         }
         stream << "\n";
      }
      stream << "\n";
      return( stream );
   }
};


/** generic base class with two ports of type port_t **/
template < typename T,
           std::size_t DIM > class dctbase : public kernel
{
public:
   using port_t = raft::matrix< T, DIM >;
   
   dctbase()
   {
      input.addPort< port_t >( "0" );
      output.addPort< port_t >( "0" );
   }
};

/** specialization base dct for 8x8 **/
template < typename T > class dctbase< T, 8 > : public kernel
{
public:
   using port_t = raft::matrix< T, 8>;
   
   dctbase()
   {
      input.addPort< port_t >( "0" );
      output.addPort< port_t >( "0" );
   }

protected:
   /**
    * dct_generic - very generic DCT for JPEG encode
    * meant to be able to be used with any fundamental
    * type and be relatively efficient.
    * @param   a  - const T*, src array
    * @param   b  - const T*, dst array 
    */
   void dct_generic( const T * const a, T * const b)
   {
      const auto height( 8 ), width( 8 );
      for( auto x( 0 ); std::isless( x, width ); x++ )
      {
         for( auto y( 0 ); std::isless( y, height ); y++ )
         {                  
            b[(x * width) + y] = compute_dct(x,y,a);      
         }
      }
   }

   /**
    * compute_dct - helper function for above generic
    * dct.  
    * @param   u - row position
    * @param   v - column position
    * @param   a - const T * const, source array
    * @return  T - dct value for this position
    */
   T compute_dct( const std::uint8_t u,
                  const std::uint8_t v, 
                  const T * const a)
   {
      constexpr const double pi_div_8( M_PI/8.0 );
      const auto dim( 8 );
      T out( static_cast< T >( 0 ) );
      for( auto x( 0 ); std::isless( x, dim ); x++ )
      {
         for( auto y( 0 ); std::isless( y, dim ); y++ )
         {
            out += (
               a[ ( x * dim ) + y ] * 
                  std::cos( pi_div_8 * ( x + .5 ) * u ) * 
                  std::cos( pi_div_8 * ( y + .5 ) * v )
               );
         }
      }
      auto alpha_func( []( const std::uint8_t x )
      {
         constexpr const double alpha_zero( 1 / ( M_SQRT2 * 2 ) );
         return( x > 0 ? .5 : alpha_zero );
      } );
      
      out = out * alpha_func( u ) * alpha_func( v );
      return( out );
   }
};

/** template base, no specials...not allowed **/
template < typename T, 
           DCTTYPE type, 
           class ENABLE = void > class dct{};

/** 
 * TODO: eventually we'll write specializations for all types, but
 * for now lets just go with one for all floating point types.
 */
template < typename T > class dct< T , 
      x88, 
      typename std::enable_if< std::is_fundamental< T >::value >::type  > :
         public dctbase< T, 8 >
{
   using port_t = raft::matrix< T, 8>;
public:
   dct() : dctbase< T, 8 >()
   {
   }

   raft::kstatus run()
   {
      auto &in_matrix( (this)->input[ "0" ]. template peek< port_t >() );
      auto &out_matrix( (this)->output[ "0" ]. template allocate< port_t >() );
      (this)->dct_generic( in_matrix.arr, out_matrix.arr ); 
      (this)->input["0"].recycle();
      (this)->output["0"].send();
      return( raft::proceed );
   }
};


} /** end namespace raft **/
#endif /* END _DCT_TCC_ */
