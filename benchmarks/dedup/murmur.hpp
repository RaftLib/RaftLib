/**
 * murmur.hpp - 
 * @author: Jonathan Beard
 * @version: Thu Jul 30 20:34:03 2015
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
#ifndef _MURMUR_HPP_
#define _MURMUR_HPP_  1
#include <cstdint>

class murmur
{
public:
   murmur() = delete;
  
   /** 
    * murmur3_32 - code for this function
    * pretty much the same as that found
    * on wikipedia, although I've found the
    * exact same code in multiple people's
    * murmur implementations.  Original
    * was C, I've modified it to C++.
    * @param key, char*
    * @param len, key length
    * @param seed - random seed
    */
   static 
   std::uint32_t 
   murmur3_32( const char * const key, 
               const std::uint32_t len, 
               const std::uint32_t seed ); 

};
#endif /* END _MURMUR_HPP_ */
