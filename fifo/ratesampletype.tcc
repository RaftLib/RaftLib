/**
 * ratesampletype.tcc - 
 * @author: Jonathan Beard
 * @version: Sat Aug 23 16:56:48 2014
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
#ifndef _RATESAMPLETYPE_TCC_
#define _RATESAMPLETYPE_TCC_  1
#include <string>
#include "ringbufferbase.tcc"
#include "ringbuffertypes.hpp"

#if __MMX__ == 1
typedef std::int64_t _v2di __attribute__((vector_size (16)));
#endif 

template < class T, Type::RingBufferType type > class RateSampleType : 
   public SampleType< T, type >
{
public:
RateSampleType() : SampleType< T, type >()
{
}

virtual ~RateSampleType()
{

}

protected:
/**
 * print_data - basic virtual function that can be used for service
 * and arrival rate functions.  
 */
virtual std::string
printData( Unit unit = Unit::Byte )
{
   return( 
   std::to_string(  
      ( (double) (this)->real.items_copied * (double) sizeof( T ) *  
         unit_conversion[ unit ] ) / 
            ( (double) (this)->real.frame_count * (double) (this)->frame_width )
   )
   );
}

/** stats struct for rates type samples **/
struct stats
{
   stats() : items_copied( 0 ),
             frame_count( 0 )
   {
   }
   
   stats( const std::int64_t frame_init ) : 
             items_copied( 0 ),
             frame_count( frame_init )
   {
   }

   stats&
   operator += (const stats &rhs )
   {
#if __MMX__ == 1
      (this)->all += rhs.all;
#else
      (this)->items_copied += rhs.items_copied;
      (this)->frame_count  += 1;
#endif
      return( *this );
   }

#if __MMX__ == 1
   union
   {
#endif   
      struct
      {
         std::int64_t items_copied;
         std::int64_t frame_count;
      };
#if __MMX__ == 1
      _v2di all;
   };
#endif
} real, temp = { 1 };
};
#endif /* END _RATESAMPLETYPE_TCC_ */
