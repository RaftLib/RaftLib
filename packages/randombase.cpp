/**
 * randombase.cpp - 
 * @author: Jonathan Beard
 * @version: Sun Feb 22 10:41:44 2015
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
#include "randombase.hpp"
#include <errno.h>
#include <cstdio>
#include <gsl/gsl_randist.h>

using namespace raft;

randombase::randombase( const raft::rndgenerator gen,
                        const std::uint64_t      seed )
{
   gsl_rng_env_setup();
}

randombase::randombase( const raft::rndgenerator gen )
{
   gsl_rng_env_setup();
}

std::uint64_t
randombase::sample_system_rng()
{
   errno = 0;
   std::uint64_t output( 0 );
   FILE *fp( fopen( "/dev/random", "r" ) );
   if( fp == nullptr )
   {
      /** TODO, throw appropriate raft exception **/
      perror( "Failed to open /dev/random, seeding with zero" );
      return( output );
   }
   if( fread( &output, sizeof( std::uint64_t ), 1, fp ) != 1 )
   {
      perror( "Failed to read from /dev/random" );
   }
   fclose( fp );
   return( output );
}
