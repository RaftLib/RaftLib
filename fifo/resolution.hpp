/**
 * resolution.hpp - 
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
#ifndef _RESOLUTION_HPP_
#define _RESOLUTION_HPP_  1
#include <cstdint>

#include "ringbufferbase.tcc"
#include "ringbuffertypes.hpp"
#include "Clock.hpp"

#ifndef NUMFRAMES
#define NUMFRAMES 5
#endif

#ifndef CONVERGENCE
#define CONVERGENCE .005
#endif

extern Clock *system_clock;

class frame_resolution
{
public:
   frame_resolution();
   virtual ~frame_resolution() = default;

   /**
    * setBlockedStatus - sets the blocked status, which will be
    * used to optimize the interval for data collection.  Direction
    * is needed so we know which side (the arrival process or 
    * service process) is actually blocked which might be used
    * seperately in the future.
    * @param   frame - current frame_resolution object
    * @param   dir   - direction of current stat selection
    * @param   blocked - true if blocked, false if not
    */
   static void setBlockedStatus(  frame_resolution &frame,
                                 Direction         dir,
                                 const bool blocked = false );
  
   /**
    * wasBlocked - returns true if at any time in the 
    * previous NUMFRAMES the queue was blocked.
    * @param   frame - frame resolution reference.
    * @return  bool - true if the queue was blocked in NUMFRAMES
    */
   static bool wasBlocked(  frame_resolution &frame );
   
   /**
    * updateResolution - function gets called at each iteration 
    * of the monitor loop until convergence is reached at which 
    * point this function returns true.
    * @param   frame - frame_resolution struct
    * @param   realized_frame_time - actual time to go through loop
    * @return  bool
    */
   static bool updateResolution(  frame_resolution          &frame,
                                  sclock_t                   previous_loop_start,
                                  bool                      &blocked );
   /**
    * acceptEntry - accept the current frame in frame if the realized 
    * interval for this observation is within the expected range.
    * @param   frame -  frame_resolution
    * @param   realized_frame_time - actual current time
    * @return  bool if acceptable
    */
   static bool acceptEntry(  frame_resolution &frame,
                            sclock_t                   realized_frame_time );

 
   static void waitForInterval(  frame_resolution &frame );
   
   sclock_t getFrameWidth();


private:
   sclock_t          curr_frame_width;
   /** might be faster with a bit vector **/
   std::int32_t      curr_frame_index;
   struct
   {
      double upper;
      double lower;
   }range;
   std::uint16_t     frame_success;
   std::uint16_t     frame_failure;
   std::uint16_t     blocked_count;
   std::uint16_t     frame_count;
};
#endif /* END _RESOLUTION_HPP_ */
