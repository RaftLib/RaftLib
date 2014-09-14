/**
 * port_info.hpp - 
 * @author: Jonathan Beard
 * @version: Wed Sep  3 20:22:56 2014
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
#ifndef _PORT_INFO_HPP_
#define _PORT_INFO_HPP_  1
#include "fifo.hpp"

struct PortInfo
{
   PortInfo( const type_info &the_type )  : typez( the_type )
   {

   }

   virtual ~PortInfo()
   {
      if( fifo != nullptr )
      {
         delete( fifo );
      }
   }

   FIFO            *fifo = std::nullptr;
   /** 
    * the type of the port.  regardless of if the buffer itself
    * is impplemented or not. 
    */
   const type_info &type;

   /**
    * const_map - stores "builder" objects for each of the 
    * currenty implemented ring buffer types so that when 
    * the mapper is allocating ring buffers it may allocate
    * one with the proper type.  The first key is self explanatory
    * for the most part, storing the ring buffer type.  The 
    * second internal map key is "instrumented" vs. not.
    */
   std::map< RingBufferType , 
             std::map< bool, std::function< FIFO* (size_t) > >* > const_map;
};
#endif /* END _PORT_INFO_HPP_ */
