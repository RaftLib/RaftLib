/**
 * scheduleconst.hpp - 
 * @author: Jonathan Beard
 * @version: Fri May 22 15:21:25 2015
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
#ifndef _SCHEDULECONST_HPP_
#define _SCHEDULECONST_HPP_  1

namespace ScheduleConst
{

   /**
    * PREEMPT_LIMIT - this is set as a global constant to 
    * preempt kernels within the fifo itself if they are
    * stuck waiting for their specified action for N or 
    * more iterations.  Eventually I'd like this to be a
    * parameter selected via some analytic formula or ML
    * but for not it'll be a static constant set by the 
    * library developer.
    */
   static const std::uint8_t PREEMPT_LIMIT = 5;
}

#endif /* END _SCHEDULECONST_HPP_ */
