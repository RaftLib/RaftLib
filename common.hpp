/**
 * common.hpp - static helper functions of various types 
 * @author: Jonathan Beard
 * @version: Sun May 10 19:10:06 2015
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
#ifndef _COMMON_HPP_
#define _COMMON_HPP_  1
#include <string>
#include <cxxabi.h>
#include <cstdlib>
#include <cassert>
#include <functional>
#include <typeinfo>

namespace common
{

/**
 * __printClassName - helper function for below function,
 * basically see the more complete docs below for the delta,
 * the string passed to this function should be the name
 * of the class from either the typeinfo or typeid( xx ).name()
 * call.
 */
static std::string __printClassName( const std::string &&obj_name )
{
   enum FailureCods : std::int32_t { InvalidArg = -3, 
                                     InvalidName = -2,
                                     MemAllocationFailure = -1,
                                     DemangleSuccess = 0 };
   std::int32_t status( 0 );
   /** user must delete this, make string then delete **/
   char *str( abi::__cxa_demangle( obj_name.c_str(), 0, 0, &status ) );
   std::string out_str( str );
   std::free( str );
   switch( status )
   {
      case( DemangleSuccess ):
      break; /** one good case **/
      case( InvalidArg ):
      {
         return( "invalid argument provided to cxa_demangle!" );
      }
      break;
      case( InvalidName ):
      {
         return( "invalid name provided to cxa_demangle" );
      }
      break;
      case( MemAllocationFailure ):
      {
         return( "memory allocation error, can't demangle class name" );
      }
      break;
      default:
         assert( false );
   }
   return( out_str );

}

static
std::string
printClassNameFromStr( const std::string &&str )
{
   return( std::move< std::string >( common::__printClassName( std::move( str ) ) ) );
}

/**
 * pringClassName - takes in a class reference and 
 * prints the class name using cxx-demangle.  I basically
 * got tired of typing all the error checking code over 
 * and over so here's a simplified interface for it.  
 * @param k - Class reference for which you want the class.
 * @returns std::string
 */
template < class K > inline
   std::string printClassName( K &k )
{
   return( std::move< std::string >( common::__printClassName( typeid( k ).name() ) ) );
}


} /** end namespace common **/

#endif /* END _COMMON_HPP_ */
