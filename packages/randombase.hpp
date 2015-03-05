/**
 * randombase.hpp - 
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
#ifndef _RANDOMBASE_HPP_
#define _RANDOMBASE_HPP_  1
#include <cstdint>
#include <gsl/gsl_rng.h>

namespace raft{

enum rndgenerator : std::int32_t { 
sequential,
borosh13,
coveyou,
cmrg,
fishman18,
fishman20,
fishman2x,
gfsr4,
knuthran,
knuthran2,
knuthran2002,
lecuyer21,
minstd,
mrg,
mt19937,
mt19937_1999,
mt19937_1998,
r250,
ran0,
ran1,
ran2,
ran3,
rand,
rand48,
random128_bsd,
random128_glibc2,
random128_libc5,
random256_bsd,
random256_glibc2,
random256_libc5,
random32_bsd,
random32_glibc2,
random32_libc5,
random64_bsd,
random64_glibc2,
random64_libc5,
random8_bsd,
random8_glibc2,
random8_libc5,
random_bsd,
random_glibc2,
random_libc5,
randu,
ranf,
ranlux,
ranlux389,
ranlxd1,
ranlxd2,
ranlxs0,
ranlxs1,
ranlxs2,
ranmar,
slatec,
taus,
taus2,
taus113,
transputer,
tt800,
uni,
uni32,
vax,
waterman14,
zuf };


class randombase
{
public:
   /**
    * randombase - initialize random number generator, provide
    * a seed.  useful for debugging, however for a more random
    * source it is advisable to use the non-seeded version which
    * uses system entropy.  The first parameter is the random 
    * number generator.  For the moment these are entirely 
    * sourced from GNU GSL so feel free to consult their docs
    * here (http://goo.gl/tfElhG).  
    * @param   gen  - raft::rndgenerator
    * @param   seed - const std::uint64_t
    */
   randombase( const raft::rndgenerator gen, const std::uint64_t seed );
   /**
    * randombase - initialize random number generator, uses
    * system entropy for a more random seed to the random number
    * generator.  The first parameter is the random number 
    * generator.  For the moment these are entirely sourced 
    * from GNU GSL so feel free to consult their docs
    * here (http://goo.gl/tfElhG).  
    */
   randombase( const raft::rndgenerator gen );

protected:
   /**
    * sample_system_rng - returns system provided uint64_t from 
    * /dev/random.
    * @return std::uint64_t 
    */
   static std::uint64_t sample_system_rng();
   
   /** vars **/
   gsl_rng *rng = nullptr;
};

}
#endif /* END _RANDOMBASE_HPP_ */
