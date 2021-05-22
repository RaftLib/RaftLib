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
#ifndef RAFTPARALLELK_HPP
#define RAFTPARALLELK_HPP  1
#if LIBCOMPILE == 1
#include "kernel.hpp"
#else
#include <raft>
#endif
#include <cstddef>

class Schedule;

namespace raft
{
class map;

class parallel_k : public raft::kernel 
{
public:
   parallel_k();
   parallel_k( void * const ptr, 
               const std::size_t nbytes );

   virtual ~parallel_k();

protected:
   /** 
    * addPort - adds a port, either to the input or 
    * output depending on what the sub-class type is
    */
   template < class T >
      std::size_t addPortTo( Port &port )
   {
      const auto portid( port_name_index++ );

#ifdef STRING_NAMES         
      port.addPort< T >( std::to_string( portid ) );
#else
      /**
       * if not strings, the addPort function expects a port_key_name_t struct,
       * so, we have to go and add it. 
       */
      port.addPort< T >( raft::port_key_name_t( portid, std::to_string( portid ) ) );
#endif
      return( portid );
   }
   
   void lock_helper( Port &port );

   void unlock_helper( Port &port );

   std::size_t  port_name_index = 0; 
   friend class ::Schedule;
   friend class map;
};

}
#endif /* END RAFTPARALLELK_HPP */
