/**
 * splitmethod.hpp - 
 * @author: Jonathan Beard
 * @version: Tue Oct 28 12:56:43 2014
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
#ifndef _SPLITMETHOD_HPP_
#define _SPLITMETHOD_HPP_  1

#include "signalvars.hpp"
#include "port.hpp"
#include "fifo.hpp"

class splitmethod 
{
public:
   splitmethod()          = default;
   virtual ~splitmethod() = default;
   
   template < class T /* item */ >
      bool send( T &item, const raft::signal signal, Port &outputs )
   {
      auto * const fifo( select_fifo( outputs, sendtype ) );
      if( fifo != nullptr )
      {
         fifo->push( item, signal );
         return( true );
      }
      else
      {
         return( false );
      }  
   }

   template < class T /* item */ >
      bool get( T &item, raft::signal &signal, Port &inputs )
   {
      auto * const fifo( select_fifo( inputs, gettype ) );
      if( fifo != nullptr )
      {
         fifo->pop< T >( item, &signal );
         return( true );
      }
      else
      {
         return( false );
      }
   }
protected:
   enum functype { sendtype, gettype };
   virtual FIFO*  select_fifo( Port &port_list, const functype type ) = 0;
};
#endif /* END _SPLITMETHOD_HPP_ */
