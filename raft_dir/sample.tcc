/**
 * sample.hpp - 
 * @author: Jonathan Beard
 * @version: Thu Aug 21 09:44:44 2014
 * 
 * Copyright 2014 Jonathan Beard
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
#ifndef _SAMPLE_HPP_
#define _SAMPLE_HPP_  1

#include <functional>
#include <vector>
#include <sstream>

#include "ringbuffertypes.hpp"
#include "ringbufferbase.tcc"
#include "sampletype.tcc"
#include "resolution.hpp"

extern Clock *system_clock;

template < class T, Type::RingBufferType type > class Sample
{
public:
Sample()
{
   sample_list = new std::vector< SampleType< T, type >* >();
}

virtual ~Sample()
{
   delete( sample_list );
   sample_list = nullptr;
}

void registerSample( SampleType< T, type >  *function )
{
   assert( function != nullptr );
   sample_list->push_back( function );
}

std::string printAllData( const char delim )
{
   std::stringstream ss;
   for( SampleType< T, type > *s : *sample_list )
   {
      s->print( ss ) << delim;
   }
   return( ss.str() );
}

static void 
run( RingBufferBase< T, type > &buffer,
     volatile bool                    &term,
     Sample                           &self )
{
   sclock_t prev_time( system_clock->getTime() );
   bool converged( false );
   bool blocked(   false );
   frame_resolution  resolution;
   while( ! term )
   {
      frame_resolution::waitForInterval( resolution );
      for( SampleType< T, type > *s : *(self.sample_list) )
      {
         s->sample( buffer, blocked );
      }
      const auto end_time( system_clock->getTime() );
      if( frame_resolution::acceptEntry( resolution,
                                         ( end_time - prev_time ) ) )
      {
       for( SampleType< T, type > *s : *(self.sample_list) )
       {
          s->accept( converged );
       }
      }
      if( ! converged )
      {
         converged = 
            frame_resolution::updateResolution( resolution, 
                                                prev_time,
                                                blocked );
         if( converged )
         {
            for( SampleType< T, type > *s : *(self.sample_list) )
            {
               s->setFrameWidth( resolution.getFrameWidth() );
            }
         }
      }
      prev_time = system_clock->getTime();
   }
}


private:
/** pointer just in case I want to use it across processes **/
std::vector< SampleType< T, type > * > *sample_list;
};
#endif /* END _SAMPLE_HPP_ */
