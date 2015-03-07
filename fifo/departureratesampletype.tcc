/**
 * departureratesampletype.tcc - 
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
#ifndef _DEPARTURERATESAMPLETYPE_TCC_
#define _DEPARTURERATESAMPLETYPE_TCC_  1
#include <string>

#include <queue>
#ifdef _EXP_
#include <array>
#include <cinttypes>
#endif

#include "ringbuffertypes.hpp"
#include "ratesampletype.tcc"
#include "blocked.hpp"
#include "Clock.hpp"

extern Clock *system_clock;

template < class T, Type::RingBufferType type > 
   class DepartureRateSampleType : public RateSampleType< T, type >
{
public:
DepartureRateSampleType() : RateSampleType< T, type >(),
                            blocked( false ),
                            finished( false ),
                            window( 16 ),
                            filtered( 12 )
{
}

virtual ~DepartureRateSampleType()
{
}

virtual void
sample( RingBufferBase< T, type > &buffer, bool &global_blocked )
{
   Blocked departure_copy;
   buffer.get_zero_read_stats( departure_copy );

#ifdef _EXP_
   (this)->temp.items_copied = departure_copy.count;
#endif
   
   if( departure_copy.blocked != 0 )
   {
      (this)->blocked = true;
      global_blocked  = true;
   }
   buffer.get_write_finished( (this)->finished );
}

virtual void
accept( volatile bool &converged )
{
   
   if( converged &&  ! (this)->blocked && ! (this)->finished )
   {
      (this)->real += (this)->temp;
      if( window.size() == 16 )
      {
         window.pop();
      }
      window.push( (this)->temp.items_copied );

#ifdef _EXP_      
      if( item_index < 100000 )
      {
         item_log[ item_index ].items = (this)->temp.items_copied;
         item_log[ item_index++ ].time  = system_clock->getTime();
      }
#endif
   }
   (this)->temp.items_copied  = 0;
   (this)->blocked            = false;
}

protected:
virtual std::string
printHeader()
{
/** below code for experimental purposes **/
#ifdef _EXP_
   std::cerr.precision( 20 );
   std::cerr << "{" << (this)->frame_width << ",{"; 
   for( auto i( 0 ); i < item_index; i++ )
   {
      if( i != (item_index - 1) )
      {
         std::cerr << "{" << item_log[ i ].time
         << "," << 
         item_log[ i ].items << "},";
      }
      else
      {
         std::cerr << "{" << item_log[ i ].time
         << "," << 
         item_log[ i ].items << "}";
      }
   }
   std::cerr << "}";
#endif
   return( "departure_rate" );
}

private:
bool  blocked;
bool  finished;


/** below code for experimental purposes **/
#ifdef __EXP__
struct ITEM
{
   std::int64_t items;
   double       time;
};
std::array< ITEM , 100000 > item_log;
std::int64_t item_index = 0;
#endif
};
#endif /* END _DEPARTURERATESAMPLETYPE_TCC_ */
