/**
 * randomstring.tcc - 
 * @author: Jonathan Beard
 * @version: Thu May  1 14:24:06 2014
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
#ifndef _RANDOMSTRING_TCC_
#define _RANDOMSTRING_TCC_  1
#include <string>
#include <cstdint>
#include <cstring>
#include <sstream>
#include <iostream>
#include <gsl/gsl_rng.h>
#include <cmath>

template < int N > class RandomString
{
public:
   RandomString() : length( 62 ),
                    t( gsl_rng_default )
   {
      source = new char[ length ];
      int8_t index( 0 );
      for( char i( 48 ); i < 58; i++ )
      {
         source[ (index++) ] = i;
      }
      for( char i( 65 ); i < 91; i++ )
      {
         source[ (index++) ] = i;
      }
      for( char i( 97 ); i < 123; i++ )
      {
         source[ (index++) ] = i;
      }
      /** setup rng **/
      errno = 0;
      FILE *fp = fopen( "/dev/urandom", "r" );
      if( fp == nullptr )
      {
         perror("Couldn't read /dev/urandom.");
         exit( EXIT_FAILURE );
      }
      uint64_t randomSeed( 0 );
      errno = 0;
      if( fread( &randomSeed, 
                 sizeof( uint64_t ),
                 1,
                 fp ) != 1 )
      {
         perror( "Error reading from /dev/urandom." );
         fclose( fp );
         exit( EXIT_FAILURE );
      }
      fclose( fp );
      gsl_rng_env_setup();
      r = gsl_rng_alloc( t);
      gsl_rng_set( r, randomSeed );
   }
   virtual ~RandomString()
   {
      delete( source );
      source = nullptr;
      gsl_rng_free( r );
   }

   std::string
   get()
   {
      char output[ N + 1 ];
      std::memset( output, '\0', N + 1 );
      for( int index( 0 ); index < N; index++ )
      {
         const int randomNumber(
            gsl_rng_uniform( r ) * (double) length );
         output[ index ] = source[ randomNumber ];
      }
      return( std::string( output ) );
   }

private:
   const int8_t       length;
   char               *source;
   gsl_rng            *r;
   const gsl_rng_type *t;
};

#endif /* END _RANDOMSTRING_TCC_ */
