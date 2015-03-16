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
#include <cstdint>

#include <gsl/gsl_randist.h>
#include <map>


using namespace raft;


std::map< raft::rndgenerator, 
                const gsl_rng_type* >
   ptrmap = 
{  
   {borosh13, gsl_rng_borosh13},
   {coveyou, gsl_rng_coveyou},
   {cmrg, gsl_rng_cmrg},
   {fishman18, gsl_rng_fishman18},
   {fishman20, gsl_rng_fishman20},
   {fishman2x, gsl_rng_fishman2x},
   {gfsr4, gsl_rng_gfsr4},
   {knuthran, gsl_rng_knuthran},
   {knuthran2, gsl_rng_knuthran2},
   {knuthran2002, gsl_rng_knuthran2002},
   {lecuyer21, gsl_rng_lecuyer21},
   {minstd, gsl_rng_minstd},
   {mrg, gsl_rng_mrg},
   {mt19937, gsl_rng_mt19937},
   {mt19937_1999, gsl_rng_mt19937_1999},
   {mt19937_1998, gsl_rng_mt19937_1998},
   {r250, gsl_rng_r250},
   {ran0, gsl_rng_ran0},
   {ran1, gsl_rng_ran1},
   {ran2, gsl_rng_ran2},
   {ran3, gsl_rng_ran3},
   {raft::rand, gsl_rng_rand},
   {rand48, gsl_rng_rand48},
   {random128_bsd, gsl_rng_random128_bsd},
   {random128_glibc2, gsl_rng_random128_glibc2},
   {random128_libc5, gsl_rng_random128_libc5},
   {random256_bsd, gsl_rng_random256_bsd},
   {random256_glibc2, gsl_rng_random256_glibc2},
   {random256_libc5, gsl_rng_random256_libc5},
   {random32_bsd, gsl_rng_random32_bsd},
   {random32_glibc2, gsl_rng_random32_glibc2},
   {random32_libc5, gsl_rng_random32_libc5},
   {random64_bsd, gsl_rng_random64_bsd},
   {random64_glibc2, gsl_rng_random64_glibc2},
   {random64_libc5, gsl_rng_random64_libc5},
   {random8_bsd, gsl_rng_random8_bsd},
   {random8_glibc2, gsl_rng_random8_glibc2},
   {random8_libc5, gsl_rng_random8_libc5},
   {random_bsd, gsl_rng_random_bsd},
   {random_glibc2, gsl_rng_random_glibc2},
   {random_libc5, gsl_rng_random_libc5},
   {randu, gsl_rng_randu},
   {ranf, gsl_rng_ranf},
   {ranlux, gsl_rng_ranlux},
   {ranlux389, gsl_rng_ranlux389},
   {ranlxd1, gsl_rng_ranlxd1},
   {ranlxd2, gsl_rng_ranlxd2},
   {ranlxs0, gsl_rng_ranlxs0},
   {ranlxs1, gsl_rng_ranlxs1},
   {ranlxs2, gsl_rng_ranlxs2},
   {ranmar, gsl_rng_ranmar},
   {slatec, gsl_rng_slatec},
   {taus, gsl_rng_taus},
   {taus2, gsl_rng_taus2},
   {taus113, gsl_rng_taus113},
   {transputer, gsl_rng_transputer},
   {tt800, gsl_rng_tt800},
   {uni, gsl_rng_uni},
   {uni32, gsl_rng_uni32},
   {vax, gsl_rng_vax},
   {waterman14, gsl_rng_waterman14},
   {zuf,gsl_rng_zuf}
};

randombase::randombase( const raft::rndgenerator gen,
                        const std::uint64_t      seed )
{
   if( gen != raft::dummy )
   {
      gsl_rng_env_setup();
      rng =  gsl_rng_alloc( ptrmap[ gen ] );
      gsl_rng_set( rng, seed ); 
   }
}

randombase::randombase( const raft::rndgenerator gen )
{
   if( gen != raft::dummy )
   {
      gsl_rng_env_setup();
      rng =  gsl_rng_alloc( ptrmap[ gen ] );
      gsl_rng_set( rng, sample_system_rng() ); 
   }
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
