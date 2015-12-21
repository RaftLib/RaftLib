/**
 * random.tcc - various specializations for random number distributions,
 * these draw heavily on the GNU GSL random number library.  Eventually
 * we'll migrate away from the GNU GSL to eliminate the library dependency,
 * however at the moment there's no point in re-engineering something
 * that isn't broken and readily available.  The important thing
 * is that the user interface doesn't rely on GNU GSL specific variables
 * so that we can swap out at some future point.
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

namespace raft
{

/** UNIFORM **/
template< class T, 
          rndtype type = raft::uniform > class random_variate : 
   public raft::randomx< T >
{
public:
   /**
    * random_variate - constructor for uniform distribution
    * @param gen - raft::rndgenerator type
    * @param seed - const std::uint64_t user provided seed
    */
   random_variate( const T min,
                   const T max,
                   const std::uintmax_t count,
                   const std::uint64_t seed, 
                   const raft::rndgenerator gen = raft::mt19937
                                    ) : randomx< T >( count, gen, seed ),
                                        min( min ),
                                        diff( max - min )
   {
      assert( max > min );
   }

   /**
    * random_variate - constructor for uniform distribution
    * @param   gen - raft::rndgenerator, generator type
    */
   random_variate( const T min,
                   const T max,
                   const std::uintmax_t count,
                   const raft::rndgenerator gen = raft::mt19937 ) 
                                          : randomx< T >( count, gen ),
                                            min( min ),
                                            diff( max - min )
   {
      
   }

   virtual ~random_variate()
   {
      /** nothing to do here **/
   }


protected:
   virtual T draw()
   {
      const auto val( gsl_rng_uniform( (this)->rng ) );
      if( almost_equal( val, 
                        static_cast< typename 
                           std::remove_reference< decltype( val ) >::type >( 0 ),
                        2 ) )
      {
         return( min );
      }
      else
      {
         return( min + (diff * val) );
      }
   }
private:
   const T min;
   const T diff;

   /**
    * NOTE: used from example here: 
    * http://en.cppreference.com/w/cpp/types/numeric_limits/epsilon,
    * couldn't have written it better myself.
    */
   template<class D>
   typename std::enable_if< ! std::numeric_limits<D>::is_integer, bool>::type
       almost_equal(const D x, const D y, const std::uint32_t ulp)
   {
       // the machine epsilon has to be scaled to the magnitude of the values used
       // and multiplied by the desired precision in ULPs (units in the last place)
       return( std::abs(x-y) < std::numeric_limits<D>::epsilon() * std::abs(x+y) * ulp
       // unless the result is subnormal
              || std::abs(x-y) < std::numeric_limits<D>::min() );
   }

};

/** EXPONENTIAL GEN **/
template< class T > class random_variate< T, raft::exponential >  : 
   public raft::randomx< T >
{
public:
   /**
    * random_variate - instantiate a generator for a random number
    * generator with an exponential distribution with the given
    * mean.  The default generator is the Mersenne Twister.
    * @param   mean - const double, mean of distribution
    * @param   count - const std::uintmax_t, number of values to send
    * @param   seed - const std::uint64_t user provided seed
    * @param   gen - number generator, default: raft::mt19937
    */
   random_variate( const double mean,
                   const std::uintmax_t count,
                   const std::uint64_t seed, 
                   const raft::rndgenerator gen = raft::mt19937 ) 
                                      : randomx< T >( count, gen, seed ),
                                        mean( mean )
   {
   }

   /**
    * random_variate - instantiate a generator for a random number
    * generator with an exponential distribution with the given
    * mean.  The default generator is the Mersenne Twister.  This
    * constructor uses the system /dev/random for a seed.  Be advised
    * that this might be slightly slower to initialize depending on
    * the amount of entropy in the system.
    * @param   mean - const double, mean of distribution
    * @param   count - const std::uintmax_t, number of values to send
    * @param   seed - const std::uint64_t user provided seed
    * @param   gen - number generator, default: raft::mt19937
    */
   random_variate( const double mean,
           const std::uintmax_t count,
           const raft::rndgenerator gen = raft::mt19937 
                                        ) : randomx< T >( count, gen ),
                                            mean( mean )
   {
   
   }

   virtual ~random_variate()
   {
      /** nothing to do here **/
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


/** GAUSSIAN **/
template< class T > class random_variate< T, raft::gaussian > : 
   public raft::randomx< T >
{
public:
   /**
    * random_variate - constructor for normal distribution random number generator
    * which takes a mean and sigma.  This class uses the ziggurat method
    * which is one of the fastest methods of producing variates drawn from
    * a normal distribution.
    * @param mean - const double, mean of distribution
    * @param sigma - standard deviation of distribution
    * @param gen - random number generator type
    * @param seed - std::uint64_t user provided seed
    */
   random_variate( const double mean,
                   const double sigma,
                   const std::uintmax_t count,
                   const std::uint64_t seed, 
                   const raft::rndgenerator gen = raft::mt19937
           ) : randomx< T >( count, gen, seed ),
                                        mean( mean ),
                                        sigma( sigma )
   {
   }

   /**
    * random_variate - constructor for normal distribution random number generator
    * which takes a mean and sigma.  This class uses the ziggurat method
    * which is one of the fastest methods of producing variates drawn from
    * a normal distribution.
    * @param   mean - const double, mean of distribution
    * @param   sigma - const double mean, standard deviation of distribution
    * @param   gen - raft::rndgenerator, type of random number generator
    */
   random_variate( const double mean,
                   const double sigma,
                   const std::uintmax_t count,
                   const raft::rndgenerator gen = raft::mt19937
           ) : randomx< T >( count, gen ),
                                            mean( mean ),
                                            sigma( sigma )
   {
   }
   
   /** destructor **/
   virtual ~random_variate()
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
      return( 
         static_cast< T > ( (this)->mean + 
                              gsl_ran_gaussian_ziggurat( (this)->rng,
                                                         (this)->sigma ) ) );
   }

private:
   const double mean;
   const double sigma;
};

/** SEQUENTIAL **/
template< class T > class random_variate< T, raft::sequential > : 
   public raft::randomx< T >
{
public:
   random_variate( const T delta,
                   const std::uintmax_t count ) : randomx< T >( count,
                                                                dummy,
                                                                0 ),
                                                  delta( delta )
   {
   }
   
   /** TODO, fix me...needs to check other.current for even div **/
   random_variate( const random_variate &other )
      : randomx< T >( other.count,
                      dummy,
                      0 ),
        min( other.min ),
        max( other.max ),
        delta( other.delta ),
        current( other.current )
   {
   
   }

   CLONE();

   random_variate(   const T min,
                     const T max,
                     const T delta,
                     const std::uintmax_t count ) : randomx< T >( count,
                                                                  dummy,
                                                                  0 ),
                                                    delta( delta ),
                                                    min( min ),
                                                    max( max )
   {
   }

protected:
   virtual T draw()
   {
      const T out( current );
      if( __builtin_expect( current > max - delta, 0 /* false */ )  )
      {
         current = min;
      }
      else
      {
         current = current + delta;
      }
      return( out );
   }
private:
   const T min = std::numeric_limits< T >::min();;
   const T max = std::numeric_limits< T >::max();
   const T delta = 1;
   T       current = min;
};


/** GAMMA DIST **/
template< class T > class random_variate< T, raft::gammadist > : 
   public raft::randomx< T >
{
public:
   /**
    * random_variate - constructor for gamma distribution random 
    * number generaton which takes two shape parameters, alpha and
    * beta.  Uses the Marsaglia-Tsang method via GNU GSL. 
    * @param alpha - const std::double 
    * @param beta - const std::double 
    * @param gen - random number generator type
    * @param seed - std::uint64_t user provided seed
    */
   random_variate( const double alpha,
                   const double beta,
                   const std::uintmax_t count,
                   const std::uint64_t seed, 
                   const raft::rndgenerator gen = raft::mt19937
           ) : randomx< T >( count, gen, seed ),
                                        alpha( alpha ),
                                        beta( beta )
   {
   }

   /**
    * random_variate - constructor for gamma distribution random 
    * number generaton which takes two shape parameters, alpha and
    * beta.  Uses the Marsaglia-Tsang method via GNU GSL.  Seed
    * drawn from /dev/random.
    * @param alpha - const std::double 
    * @param beta - const std::double 
    * @param gen - random number generator type
    */
   random_variate( const double alpha,
                   const double beta,
                   const std::uintmax_t count,
                   const raft::rndgenerator gen = raft::mt19937
           ) : randomx< T >( count, gen ),
                                            alpha( alpha ),
                                            beta( beta )
   {
   }
   
   /** destructor **/
   virtual ~random_variate()
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
      return( (T)(  gsl_ran_gamma( (this)->rng, alpha, beta ) ) );
   }
private:
   const double alpha;
   const double beta;
};

}
#endif /* END _RANDOM_TCC_ */
