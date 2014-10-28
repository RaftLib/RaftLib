/**
 * parallelk.hpp - 
 * @author: Jonathan Beard
 * @version: Mon Oct 20 13:18:21 2014
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
#ifndef _PARALLELK_HPP_
#define _PARALLELK_HPP_  1
#include <raft>
#include <cstddef>

class parallel_k : public Kernel 
{
public:
   parallel_k()          = default;
   virtual ~parallel_k() = default;

protected:
   /** 
    * addPort - adds a port, either to the input or 
    * output depending on what the sub-class type is
    */
   template < class T >
      void addPortTo( Port &port )
   {
      port.addPort< T >( std::to_string( port_name_index++ ) );
   }


   virtual void addPort() = 0;

   std::size_t  port_name_index = 0; 
   friend class Schedule;
   friend class Map;
};
#endif /* END _PARALLELK_HPP_ */
