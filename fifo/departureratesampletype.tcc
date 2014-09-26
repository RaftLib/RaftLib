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
#include "ringbuffertypes.hpp"
#include "ratesampletype.tcc"
#include "blocked.hpp"
#include <cinttypes>
#include "Clock.hpp"

extern Clock *system_clock;

template < class T, Type::RingBufferType type > 
   class DepartureRateSampleType : public RateSampleType< T, type >
{
public:
DepartureRateSampleType() : RateSampleType< T, type >(),
                            blocked( false ),
                            finished( false ),
                            prev_time( (sclock_t) 0 )
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
   (this)->temp.items_copied = departure_copy.count;
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
   const sclock_t curr_time( system_clock->getTime() );
   if( converged &&  ! (this)->blocked && ! (this)->finished )
   {
      (this)->real += (this)->temp;
      fprintf( stderr, "%" PRIu64 ",%.10f\n", (this)->temp.items_copied,
                                              ( curr_time - (this)->prev_time) );
   }
   (this)->temp.items_copied  = 0;
   (this)->blocked            = false;
   (this)->prev_time = system_clock->getTime();
}

protected:
virtual std::string
printHeader()
{
   return( "departure_rate" );
}

private:
bool  blocked;
bool  finished;
sclock_t prev_time;
};
#endif /* END _DEPARTURERATESAMPLETYPE_TCC_ */
