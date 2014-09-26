/**
 * Clock.cpp - 
 * @author: Jonathan Beard
 * @version: Thu May 29 14:23:18 2014
 * 
 * Copyright 2014 Jonathan Beard
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
#include <array>
#include <cstdint>
#include <iostream>
#include "Clock.hpp"

Clock::Clock() : resolution( nullptr )
{
}


Clock::~Clock()
{
   if( resolution != nullptr )
   {
      delete( resolution );
   }
   resolution = nullptr;
}


sclock_t
Clock::getResolution()
{
   if( (this)->resolution != nullptr )
   {
      return( *resolution );
   }
   /** TODO, figure out why some sizes throw a KERN_PROT_FAULT on OS X**/
   const size_t count( 1e4 );
   std::array< sclock_t , count > update_array; 
   for( size_t index( 0 ); index < count; index++ )
   {
      update_array[ index ] = getTime();
   }
   struct Mean{
      Mean() : total( 0.0 ),
               n( 0 )
      {}

      sclock_t   total;
      uint64_t n;
      
      sclock_t get()
      {
         return( total / n );
      }
      void operator += (const sclock_t val )
      {
         total += val;
         n++;
      }

      std::ostream& print( std::ostream &stream )
      {
         stream << get();
         return( stream );
      }
   } mean;

   for( size_t index( 1 ); index < count; index++ )
   {
      const sclock_t val( update_array[ index ] - update_array[ index - 1 ] );
      if( val > 1e-12 ) /** something lower than we'd expect resolution to be **/
      {
         mean += val; 
      }
   }
   resolution = new sclock_t();
   *resolution = mean.get();
   return( *resolution );
}
