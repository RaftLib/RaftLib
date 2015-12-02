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
#ifndef _RANDOMX_TCC_
#define _RANDOMX_TCC_  1
#include <cstdint>
#include <raft>
namespace raft{

enum rndtype { 
   sequential,
   exponential, 
   gaussian, 
   uniform,
   gammadist /** intel math.h header has a term gamma, could conflict **/ }; 

template < class T > class randomx : public raft::randombase, 
                                     public raft::parallel_k
{
public:
   /**
    * randomx - initialize random number generator, provide
    * a seed.  useful for debugging, however for a more random
    * source it is advisable to use the non-seeded version which
    * uses system entropy.  The first parameter is the random 
    * number generator.  For the moment these are entirely 
    * sourced from GNU GSL so feel free to consult their docs
    * here (http://goo.gl/tfElhG). 
    * @param   count - number of random numbers to generate
    * @param   gen  - raft::rndgenerator
    * @param   seed - const std::uint64_t
    */
   randomx( const std::int64_t       count,
            const raft::rndgenerator gen, 
            const std::uint64_t      seed ) : randombase( gen, seed ),
                                              count( count )
   {
      addPortTo< T >( output );
   }


   /**
    * randomx - initialize random number generator, uses
    * system entropy for a more random seed to the random number
    * generator.  The first parameter is the random number 
    * generator.  For the moment these are entirely sourced 
    * from GNU GSL so feel free to consult their docs
    * here (http://goo.gl/tfElhG).  
    */
   randomx( const std::int64_t       count,
            const raft::rndgenerator gen ) : randombase( gen ),
                                             count( count )
   {
      addPortTo< T >( output );
   }

   virtual ~randomx()
   {
   }

   /**
    * run - sends a random number to each port
    * and returns raft::proceed.
    * @return raft::kstatus
    */
   virtual raft::kstatus run()
   {
      for( auto &port : output )
      {
         port.template push< T >( std::forward< T >( draw() ) );
         if( --count <= 0 )
         {
            return( raft::stop );
         }
      }
      return( raft::proceed );
   }
protected:
   /**
    * draw - implement to create a sub-class, should
    * call a random number generator to return a new
    * variate with every call and return a variable
    * of type T.
    * @return T
    */
   virtual T draw() = 0;
   /** 
    * addPort - implementation of addPort from parallel_k
    */

   virtual std::size_t  addPort()
   {
      return( (this)->addPortTo< T >( output ) );
   }
   
   std::int64_t count = 0;
};

}

#endif /* END _RANDOMX_TCC_ */
