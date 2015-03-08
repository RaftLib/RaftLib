/**
 * stat.tcc - collection of functions and methods for 
 * stats.
 * @author: Jonathan Beard
 * @version: Fri Mar  6 15:10:36 2015
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
#ifndef _STAT_TCC_
#define _STAT_TCC_  1
#include <functional>
#include <utility>
#include <cstdint>
#include <cstddef>

class stat
{
public:
   stat() = delete;

   /**
    * mean - return simple mean of values form container c
    * @param   c - const Container &&c
    * @return R - mean cast to type R
    */
   template < typename R, typename Container >
   static
   R  mean( Container &&c )
   {
      R r( static_cast< R >( 0 ) );
      for( const auto val : c )
      {
         r += val;
      }
      return( static_cast< R >( r / c.size() ) );
   }


   /**
    * mean - computes simple mean of data set pointed to 
    * by param arr with length (elements of integral type
    * A) N.
    * @param   arr - const A const *, data set
    * @param   N - const std::size_t, size of 
    *              data set in units of sizeof(A)
    * @return  R - simple mean
    */
   template < typename R, typename A >
   static
   R mean( const A const * arr, const std::size_t N )
   {
      R r( static_cast< R >( 0 ) );
      for( auto i( 0 ); i < N; i++ )
      {
         r += arr[ i ];
      }
      return( static_cast< R >( r / N ) );
   }

   /**
    * standardDeviation - returns standard deviation of
    * the data set contained by Container c with pre-computed
    * mean.
    * @param   mean - const T, pre-computed mean of c
    * @param   c - Container&&, data set
    * @return  T - standard deviation of set based on given mean
    */
   template < typename T, typename Container >
   static
   T standardDeviation( const T mean, Container &&c )
   {
      T sumsq( static_cast< T >( 0 ) );
      for( const auto val : arr )
      {
         const auto term( val - mean );
         sumsq += ( term * term );
      }
      return( static_cast< T >( std::sqrt( sumsq ) ) );
   }

   /**
    * standardDeviation - returns standard deviation of 
    * the data set pointed to by arr, with mean mean and
    * set size N.
    * @param   mean - const T, pre-computed mean of set arr
    * @param   arr - const A const *, data set
    * @param   N - const std::size_t size of arr
    * @return T - standard deviation of set
    */
   template < typename T, typename A >
   static
   T standardDeviation( const T mean, const A const * arr,
                           const std::size_t N )
   {
      T sumsq( static_cast< T >( 0 ) );
      for( auto i( 0 ); i < N; i++ )
      {
         const auto term( arr[ i ] - mean );
         sumsq += ( term * term );
      }
      return( static_cast< T >( std::sqrt( sumsq ) ) );
   }
   
   /** 
    * z_standardize - performs standardization of the data
    * set c so that the resultant set has a mean of zero and
    * standard deviation of 1.
    * @param   c - Container&&, data set to standardize
    * @param   mean - current mean of set c
    * @param   sigma - current standard deviation of  c
    */
   template < typename Container, typename M, typename SD >
   static
   void z_standardize( Container &&c,
                       const M  mean,
                       const SD sigma )
   {
      for( auto &val : c )
      {
         val = ( val - mean ) / sigma;
      }
   }

   /**
    * zero_one_scale - scales all data in set c so that the
    * resultant range is between zero and one.
    * @param   c - Container&&, data set to scale
    * @return  std::pair< T, T >, min and max of set
    */

   template < class Container >
   static
   auto zero_one_scale( Container &&c ) ->
      std::pair< 
         typename std::remove_reference< decltype( (*c.begin()) ) >::type,
         typename std::remove_reference< decltype( (*c.begin()) ) >::type >
   {
      using c_type = typename std::remove_reference< decltype( (*c.begin()) )>::type;
      c_type min( std::numeric_limits< c_type >::max() );
      c_type max( std::numeric_limits< c_type >::min() );
      for( const auto val : c )
      {
         if( val < min )
         {
            min = val;
         }
         if( val > max )
         {
            max = val;
         }
      }
      const auto denominator( max - min );
      for( auto &val : c )
      {
         val = ( val - min ) / denominator;
      }
      return( std::make_pair( min, max ) );
   }

   /**
    * min_max_scale - scales the data based upon the min and max
    * of the given data set where each element is x / (max x - min x)
    * @param c - Container&&
    * @return std::pair< T, T >, where T is the type contained with
    *         c and the first corresponds to the min and second 
    *         corresponds to the max.
    */
   template < class Container >
   static
   auto min_max_scale( Container &&c ) ->
      std::pair< 
         typename std::remove_reference< decltype( (*c.begin()) ) >::type,
         typename std::remove_reference< decltype( (*c.begin()) ) >::type >
   {
      using c_type = typename std::remove_reference< decltype( (*c.begin()) )>::type;
      c_type min( std::numeric_limits< c_type >::max() );
      c_type max( std::numeric_limits< c_type >::min() );
      for( const auto val : c )
      {
         if( val < min )
         {
            min = val;
         }
         if( val > max )
         {
            max = val;
         }
      }
      const auto denominator( max - min );
      for( auto &val : c )
      {
         val = val / denominator;
      }
      return( std::make_pair( min, max ) );
   }

   /**
    * sigma_scale - scale a data set by its standard deviation
    * according to the formula: x / sigma
    * @param c - Container&&, data set to scale
    * @param sigma - standard deviation of set
    */
   template< class Container,
             typename S >
   static
   void sigma_scale( Container &&c, const S sigma )
   {
      for( auto &val : c )
      {
         val = val / sigma;
      }
      return;
   }

   /**
    * scale_to_range - scale a data set by a specific range
    * @param c - Container&&, data set to scale
    */
   template < class Container, typename A, typename B >
   static
   void scale_to_range( Container &&c, const A a, const B b )
   {
      using c_type = typename std::remove_reference< decltype( (*c.begin()) )>::type;
      c_type min( std::numeric_limits< c_type >::max() );
      c_type max( std::numeric_limits< c_type >::min() );
      for( const auto val : c )
      {
         if( val < min )
         {
            min = val;
         }
         if( val > max )
         {
            max = val;
         }
      }
      const auto range( b - a );
      const auto denominator( max - min );
      for( auto &val : c )
      {
         val = ( ( range * ( val - min ) ) / denominator ) + a;
      }
      return;
   }
};
#endif /* END _STAT_TCC_ */
