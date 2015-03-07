/**
 * ratesample.hpp - this class encapsulates the logic for arrival and 
 * departure rate sampling. 
 *
 * @author: Jonathan Beard
 * @version: Fri Mar  6 11:37:08 2015
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
#ifndef _RATESAMPLE_HPP_
#define _RATESAMPLE_HPP_  1
#include <queue>
#include <cstddef>
#include <cstdint>
/** for mean / std-dev **/
#include "streamingstat.tcc"
/** for system clock **/
#include "Clock.hpp"
class ratesample 
{
/**
 * WINDOW_SIZE - number of counts, added through addCount() to keep
 */
const std::uint8_t WINDOW_SIZE   = 16;
/**
 * G_FILTER_RADIOUS - radius of Gaussian filter
 */
const std::uint8_t G_FILTER_RADIUS = 2;
/**
 * FILTERED_WINDOW_SIZE - size of post-filter since we're not padding the 
 * data the width is 2x filter radius of the window size.
 */
const std::uint8_t FILTERED_WINDOW_SIZE = WINDOW_SIZE - ( 2 * G_FILTER_RADIUS );

public:
   /**
    * ratesample - instantiate a rate sample.  the only parameter
    * is that of the frame width which is needed to calculate the 
    * rate.
    * @
    */
   ratesample( const sclock_t    frame_width,
               const std::size_t ele_size );

   /** destructor **/
   virtual ~ratesample();


   /**
    * addCount- adds a new count to the window,
    * function discards previous observation 
    * over the window size.  This also has the 
    * consequence of applying the filter and 
    * updating the current estimate of the 
    * rate.
    * @param   value - const std::uint32_t
    */
   void addCount( const std::uint32_t value );

   /**
    * getRate - returns the current estimate
    * of the rate.  Might change precision in
    * the future, but it seems the accuracy of
    * the method isn't nearly precise enough
    * to benefit from double precision.
    * @return float
    */
   float getRate();

private:
   /**
    * update - called internally to update the 
    * streaming estimate of the rate.
    */
   void update();

   std::queue< std::uint32_t >               window;
   std::queue< float >                       filtered;
   const sclock_t                            frame_width;
   const std::size_t                         ele_size;
   streamingstat< float >                    stats;
   filter< float, 2, Gaussian >              gauss_filter;
   filter< double, 1, LaplacianGaussian >    laplac_filter;
   float                                     rate;
};
#endif /* END _RATESAMPLE_HPP_ */
