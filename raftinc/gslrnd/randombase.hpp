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
#ifndef RAFTRANDOMBASE_HPP
#define RAFTRANDOMBASE_HPP  1
#include <cstdint>

namespace raft{



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
    * /dev/urandom.  Normally I'd say use /dev/random however
    * then we have to handle blocking for lack of entropy and this
    * seems a bit simpler at the moment.
    * @return std::uint64_t 
    */
   static std::uint64_t sample_system_rng();
   
   /** vars **/
   gsl_rng *rng = nullptr;
};

}
#endif /* END RAFTRANDOMBASE_HPP */
