/**
 * resolution.cpp - 
 * @author: Jonathan Beard
 * @version: Wed Aug 20 12:53:16 2014
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
#include "resolution.hpp"

#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <cassert>
#include <functional>

frame_resolution::frame_resolution() :
                                       curr_frame_width( 0 ),
                                       curr_frame_index( 0 ),
                                       frame_success( 0 ),
                                       frame_failure( 0 ),
                                       blocked_count( 0 ),
                                       frame_count( 0 )
{
   assert( system_clock != nullptr );
   curr_frame_width = system_clock->getResolution(); 
}


/**
 * updateResolution - function gets called at each iteration 
 * of the monitor loop until convergence is reached at which 
 * point this function returns true.
 * @param   frame - frame_resolution struct
 * @param   realized_frame_time - actual time to go through loop
 * @return  bool
 */
bool 
frame_resolution::updateResolution(  frame_resolution &frame,
                                     sclock_t         previous_loop_start, 
                                     bool             &blocked )
{
   
   const auto realized_frame_time( system_clock->getTime() - previous_loop_start );
   const double p_diff( 
   ( realized_frame_time - frame.curr_frame_width ) /
      frame.curr_frame_width );
   if( ++frame.frame_count == 5 )
   {
      frame.blocked_count = 0;
   }

   if( blocked )
   {
      frame.blocked_count++;
      blocked = false;
   }
   auto update = [&]()
   {
      if( frame.frame_failure++ > 1 )
      {
         frame.curr_frame_width += system_clock->getResolution();
         frame.frame_failure = 0;
         frame.frame_success = 0;
         frame.blocked_count = 0;
      }
      return( false );
   };
   if( p_diff < ( -CONVERGENCE ) /* ||  frame.blocked_count < 2 */ )
   {
      update();
   }
   else if ( p_diff > CONVERGENCE /* || frame.blocked_count < 2 */ )
   {
      update(); 
   }
   if( frame.frame_success++ < 5 )
   {
      return( false );
   }
   //else calc range
   const double upperPercent( 1.01  );
   const double lowerPercent( .99   );
   /** note: frame.curr_frame_width always > 0 **/
   frame.range.upper = frame.curr_frame_width * upperPercent;
   frame.range.lower = frame.curr_frame_width * lowerPercent;
   
   std::cout << "frame interval: " << frame.curr_frame_width << "s\n";
   return( true );
}

bool 
frame_resolution::acceptEntry(  frame_resolution   &frame,
                                sclock_t            realized_frame_time )
{
   if( realized_frame_time >= frame.range.lower && 
         realized_frame_time <= frame.range.upper )
   {
      return( true );
   }
   return( false );
}

void 
frame_resolution::waitForInterval(  frame_resolution &frame )
{
   const sclock_t stop_time( system_clock->getTime() + frame.curr_frame_width );
   while( system_clock->getTime() < stop_time ) 
   {
#if __x86_64            
      __asm__ ("\
         pause"
         :
         :
         : );
#endif               
   }
   return;
}

sclock_t
frame_resolution::getFrameWidth()
{
   return( curr_frame_width );
}
