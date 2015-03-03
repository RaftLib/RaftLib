/**
 * random.tcc - various specializations for random number distributions,
 * these draw heavily on the GNU GSL random number library
 *
 * @author: Jonathan Beard
 * @version: Tue Mar  3 11:52:12 2015
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
#include <raftrandom>
#include <cmath>
#include <limits>
#include <iomanip>
#include <type_traits>
#include <cstdint>
#include <algorithm>
#include <gsl/gsl_randist.h>

namespace raft
{

template< enum RNDTYPE, class T > class random : public raft::randomx< RNDTYPE, T >;

template< class T > class random : public raft::randomx< raft::exponential, T >
{
public:
   random( const double mean,
           const std::uintmax_t count,
           const raft::rndgenerator gen,
           const std::uint64_t seed ) : randomx( count, gen, seed ),
                                        mean( mean )
   {
   }

   random( const double mean,
           const std::uintmax_t count,
           const raft::rndgenerator gen ) : randomx( count, gen ),
                                            mean( mean )
   {
   
   }

   virtual ~random()
   {
   }


protected:
   /**
    * draw - implement virtual function of randomx to return a 
    * an exponential produced using the GNU GSL function
    * gsl_ran_exponential cast to type (T).
    * @return T - exponential drawn from the exp. dist with mean
    * set from the constructor.
    */
   virtual T draw()
   {
      return( (T) gsl_ran_exponential( (this)->rng, (this)->mean ) );
   }

private:
   const double mean;
};

template< class T > class random : public raft::randomx< raft::gaussian, T >
{
public:
   /**
    * random - constructor for normal distribution random number generator
    * which takes a mean and sigma.  This class uses the ziggurat method
    * which is one of the fastest methods of producing variates drawn from
    * a normal distribution.
    * @param mean - const double, mean of distribution
    * @param sigma - standard deviation of distribution
    * @param gen - random number generator type
    * @param seed - std::uint64_t user provided seed
    */
   random( const double mean,
           const double sigma,
           const std::uintmax_t count,
           const raft::rndgenerator gen,
           const std::uint64_t seed ) : randomx( count, gen, seed ),
                                        mean( mean ),
                                        sigma( sigma )
   {
   }

   /**
    * random - constructor for normal distribution random number generator
    * which takes a mean and sigma.  This class uses the ziggurat method
    * which is one of the fastest methods of producing variates drawn from
    * a normal distribution.
    * @param   mean - const double, mean of distribution
    * @param   sigma - const double mean, standard deviation of distribution
    * @param   gen - raft::rndgenerator, type of random number generator
    */
   random( const double mean,
           const double sigma,
           const std::uintmax_t count,
           const raft::rndgenerator gen ) : randomx( count, gen ),
                                            mean( mean ),
                                            sigma( sigma )
   {
   }
   
   /** destructor **/
   virtual ~random()
   {
   }


protected:
   /**
    * draw - implements virtual function of randomx to return a normal
    * variable of type T drawn from the normal distribution using the 
    * gsl_ran_gaussian_ziggurat function cast to type (T) offset by 
    * the specified mean.
    * @return  T  - random var. drawn from normal distribution
    */
   virtual T draw()
   {
      return( (T)( (this)->mean + gsl_ran_gaussian_ziggurat( (this)->rng,
                                                             (this)->sigma ) ) );
   }
private:
   const double mean;
   const double sigma;
};

/** UNIFORM **/
template< class T > class random : public raft::randomx< raft::uniform, T >
{
public:
   /**
    * random - constructor for uniform distribution
    * @param gen - raft::rndgenerator type
    * @param seed - const std::uint64_t user provided seed
    */
   random( const T min,
           const T max,
           const std::uintmax_t count,
           const raft::rndgenerator gen,
           const std::uint64_t seed ) : randomx( count, gen, seed ),
                                        min( min ),
                                        diff( max - min )
   {
      assert( max > min );
   }

   /**
    * random - constructor for uniform distribution
    * @param   gen - raft::rndgenerator, generator type
    */
   random( const T min,
           const T max,
           const std::uintmax_t count,
           const raft::rndgenerator gen ) : randomx( count, gen ),
                                            min( min ),
                                            diff( max - min )
   {
      
   }

   virtual ~random()
   {
      /** nothing to do here **/
   }


protected:
   virtual T draw()
   {
      const auto val( gsl_rng_uniform( (this)->rng ) );
      if( almost_equal( val, 
                        static_cast<  std::remove_reference< decltype( val ) > >(0.0),
                        2 ) )
      {
         return( (T) min );
      }
      else
      {
         return( (T)( min + (diff * val) ) );
      }
   }
private:
   const T min;
   const T diff;

   /**
    * NOTE: used from example here: http://en.cppreference.com/w/cpp/types/numeric_limits/epsilon,
    * couldn't have written it better myself.
    */
   template<class D>
   typename std::enable_if<!std::numeric_limits<D>::is_integer, bool>::type
       almost_equal(const D x, const D y, const std::uint32_t ulp)
   {
       // the machine epsilon has to be scaled to the magnitude of the values used
       // and multiplied by the desired precision in ULPs (units in the last place)
       return std::abs(x-y) < std::numeric_limits<D>::epsilon() * std::abs(x+y) * ulp
       // unless the result is subnormal
              || std::abs(x-y) < std::numeric_limits<D>::min();
   }

};

}
#endif /* END _RANDOM_TCC_ */
