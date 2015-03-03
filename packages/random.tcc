/**
 * random.tcc - slightly more than a warpper around the GNU GSL
 * library with initialization functions to seed the rnd and 
 * stream the random numbers.
 * @author: Jonathan Beard
 * @version: Sun Feb 22 09:58:52 2015
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
#ifndef _RANDOM_TCC_
#define _RANDOM_TCC_  1

#include <raft>
namespace raft{

template < class RNDTYPE > class random : public randombase
{
   /**
    * random - initialize random number generator, provide
    * a seed.  useful for debugging, however for a more random
    * source it is advisable to use the non-seeded version which
    * uses system entropy.  The first parameter is the random 
    * number generator.  For the moment these are entirely 
    * sourced from GNU GSL so feel free to consult their docs
    * here (http://goo.gl/tfElhG).  
    * @param   gen  - raft::rndgenerator
    * @param   seed - const std::uint64_t
    */
   random( const raft::rndgenerator gen, 
           const std::uint64_t seed )
   {

   }

   /**
    * random - initialize random number generator, uses
    * system entropy for a more random seed to the random number
    * generator.  The first parameter is the random number 
    * generator.  For the moment these are entirely sourced 
    * from GNU GSL so feel free to consult their docs
    * here (http://goo.gl/tfElhG).  
    */
   random( const raft::rndgenerator gen )
   {

   }

   virtual ~random()
   {

   }
};

}

#endif /* END _RANDOM_TCC_ */
