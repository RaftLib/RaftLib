/**
 * pointer.hpp - 
 * @author: Jonathan Beard
 * @version: Thu May 15 09:58:51 2014
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
#ifndef _POINTER_HPP_
#define _POINTER_HPP_  1

#include <cstdlib>
#include <cstdint>

class Pointer{
public:
   /**
    * Pointer - used to synchronize read and write
    * pointers for the ring buffer.  This class encapsulates
    * wrapping.
    */
   Pointer( const size_t cap );

   /**
    * val - returns the current value of val.  Internally
    * the function checks to ensure that any writes to the
    * pointer have fully completed so that the value returned
    * is ensured to be the correct one.  This is important
    * as it means the producer will only see a "conservative"
    * estimate of how many items can be written and the 
    * consumer will only see a "conservative" estimate of how
    * many items can be read.
    * @return size_t, current 'true' value of the pointer
    */
   static size_t val( Pointer *ptr );

   /**
    * inc - increments the pointer, takes care of wrapping
    * the pointers as well so you don't run off the page
    * @return  size_t, current value of pointer after increment
    */
   static size_t inc( Pointer *ptr );
   
   /**
    * incBy - increments the current pointer poisition
    * by 'in' increments.  To be used for range insertion
    * and removal
    * @param  in - const size_t
    * @return  size_t, current increment after adding 'in'
    */
   static size_t incBy( const size_t in, Pointer *ptr );

   
   /**
    * wrapIndiciator - returns the current wrap position, 
    * the read should never be ahead of the write, and 
    * at best they should be equal.  This is used when
    * determining to return max_cap or zero for the current
    * queue size.  
    * @return  size_t
    */
   static size_t wrapIndicator( Pointer *ptr );
   
private:
   volatile std::uint64_t           a;
   volatile std::uint64_t           b;
   /**
    * size of wrap pointer might become an issue
    * if GHz increase drastically or if this runs
    * for a really really long time.
    * TODO, get these set correctly if we do eventually
    * wrap an unsigned 64 int.
    */
   volatile std::uint64_t    wrap_a;
   volatile std::uint64_t    wrap_b;
   const    size_t           max_cap;
};
#endif /* END _POINTER_HPP_ */
