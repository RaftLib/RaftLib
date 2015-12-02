/**
 * Clock.hpp - object for determining time across cores on a multi-core
 * processor.  You can get multiple clock types with the same interface
 * by instantiating the appropriate template.  These currently include
 * system clocks (both OS X, Linux and Unix) as well as cycle counters
 * for x86 processors (both AMD, Intel, 32 & 64-bit ).
 *
 * @author: Jonathan Beard
 * @version: Thu May 29 14:23:18 2014
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
#ifndef _CLOCK_HPP_
#define _CLOCK_HPP_  1

/** data type for clock, directly relates to resolution **/
typedef double sclock_t;

class Clock
{
public:
   /** default initializer **/
   Clock();
   /** destructor **/
   virtual ~Clock();

   /**
    * getTime - return the current time, accurate to below
    * resolution.
    * @return sclock_t, current time
    */
   virtual sclock_t getTime() = 0;

   /**
    * getResolution - returns the minimum "resolution" which 
    * might be better termed latency for this clock's update
    * takes non-zero time to compute empirically.
    * @return sclock_t, current resolution. 
    */
   sclock_t getResolution();

private:
   sclock_t   *resolution;
};
#endif /* END _CLOCK_HPP_ */
