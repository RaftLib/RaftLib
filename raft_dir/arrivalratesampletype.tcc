/**
 * arrivalratesampletype.tcc - 
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
#ifndef _ARRIVALRATESAMPLETYPE_TCC_
#define _ARRIVALRATESAMPLETYPE_TCC_  1

#include <string>

#include "ringbuffertypes.hpp"
#include "ratesampletype.tcc"
#include "blocked.hpp"
#include "Clock.hpp"
#include <cstdlib>
#include <cassert>

extern Clock *system_clock;

template < class T, Type::RingBufferType type > class ArrivalRateSampleType : 
   public RateSampleType< T, type >
{
public:
ArrivalRateSampleType() : RateSampleType< T, type >(),
                          arrival_started( false ),
                          blocked( false ),
                          finished( false ),
                          prev_time( (sclock_t) 0 )
{
}

virtual ~ArrivalRateSampleType()
{
}

virtual void
sample( RingBufferBase< T, type > &buffer,
        bool                      &global_blocked )
{
   /** nomenclature is a but funky but arrival = writes to queue **/ 
   Blocked arrival_copy;
   buffer.get_zero_write_stats( arrival_copy );
   if( ! arrival_started )
   {
      if( arrival_copy.count != 0 )
      {
         (this)->arrival_started = true;
         global_blocked          = true;
      }
      return;
   }

   (this)->temp.items_copied = arrival_copy.count;
   if( arrival_copy.blocked != 0 )
   {
      (this)->blocked = true;
   }  
   buffer.get_write_finished( finished );
}


/**
 * accept - take the current frame and add it to the overall
 * total, would be nice to have a single virtual function 
 * in the ratesampletype template, however I have a feeling
 * the logic will diverge significantly enough that this will
 * be less confusing.
 * @param   converged - true if the times have converged.
 */
virtual void
accept( volatile bool &converged )
{
   if( converged && ! (this)->blocked && (this)->arrival_started && ! (this)->finished )
   {
      (this)->real += (this)->temp;
   }
   (this)->temp.items_copied = 0;
   (this)->blocked = false;
   (this)->prev_time = system_clock->getTime();
}

protected:
virtual std::string 
printHeader()
{
   return( "arrival_rate" );
}

private:
   bool    arrival_started;
   bool    blocked;
   bool    finished;
   sclock_t prev_time;
};
#endif /* END _ARRIVALRATESAMPLETYPE_TCC_ */
