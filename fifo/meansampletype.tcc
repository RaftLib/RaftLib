/**
 * meansampletype.tcc - 
 * @author: Jonathan Beard
 * @version: Thu Aug 21 12:49:40 2014
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
#ifndef _MEANSAMPLETYPE_TCC_
#define _MEANSAMPLETYPE_TCC_  1
#include <string>

#include "ringbuffertypes.hpp"
#include "sampletype.tcc"
template< class T, Type::RingBufferType type > 
   class MeanSampleType : public SampleType< T, type >
{
public:
MeanSampleType() : SampleType< T, type >()
{

}

virtual ~MeanSampleType()
{
}


virtual void
sample( RingBufferBase< T, type > &buffer,
        bool                      &global_blocked )
{
   (this)->temp.occupancy    += buffer.size();
}

virtual void
accept( volatile bool &converged )
{
   if( converged )
   {
      (this)->real += (this)->temp;
      //std::cerr << (this)->temp.occupancy << "\n";
   }
   /** TODO, change this if I change the struct to a union **/
   (this)->temp.occupancy        = 0;
}

protected:
virtual std::string
printHeader()
{
   return( "mean_occupancy" );
}

virtual std::string
printData( Unit unit = Unit::Byte )
{
   //std::stringstream ss;
   //ss << (this)->real.occupancy << ", " << (this)->real.frames_count;
   //return( ss.str() );
   return( std::to_string( 
      (double) (this)->real.occupancy / ( (double) (this)->real.frames_count ) ) );
}

private:
/* I want to keep the additions to these from getting optimized out */
/** TODO, maybe change to a union **/
struct stats{
   stats() : occupancy( 0 ),
             frames_count( 0 )
   {
   }

   stats&
   operator += ( const stats &rhs )
   {
      (this)->occupancy    += rhs.occupancy;
      (this)->frames_count += 1;
      return( *this );
   }
   
   volatile std::int64_t occupancy;
   volatile std::int64_t frames_count;
} real, temp;

};
#endif /* END _MEANSAMPLETYPE_TCC_ */
