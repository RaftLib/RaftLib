/**
 * ratesample.cpp - 
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
#include "ratesample.hpp"

ratesample::ratesample( const sclock_t frame_width,
                        const std::size_t  ele_size ) : window( WINDOW_SIZE ),
                                                        filtered( FILTERED_WINDOW_SIZE ),
                                                        frame_width( frame_width ),
                                                        ele_size( ele_size ),
                                                        laplac_filter( .5 )
{

}


ratesample::~ratesample()
{
   /** nothing to do here **/
}

void
ratesample::addCount( const std::uint32_t value )
{
   if( window.size() == WINDOW_SIZE )
   {  
      /** update rate **/
      update();
      window.pop();
   }
   window.push( value );
}

float
ratesample::getRate()
{
   return( (float) stats.mean() );
}
