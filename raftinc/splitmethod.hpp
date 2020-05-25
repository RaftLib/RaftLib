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
#ifndef RAFTSPLITMETHOD_HPP
#define RAFTSPLITMETHOD_HPP  1

#include <type_traits>
#include <functional>

#include "autoreleasebase.hpp"
#include "signalvars.hpp"
#include "port.hpp"
#include "fifo.hpp"


class autoreleasebase;

/** FIXME, it's relativly easy to do zero copy....so implement **/
class splitmethod
{
public:
   splitmethod()          = default;
   virtual ~splitmethod() = default;

   template < class T /* item */,
              typename std::enable_if<
                        std::is_fundamental< T >::value >::type* = nullptr >
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

   /**
    * send - this version is intended for the peekrange object from
    * autorelease.tcc in the fifo dir.  I'll add some code to enable
    * only on the autorelease object shortly, but for now this will
    * get it working.
    * @param   range - T&, autorelease object
    * @param   outputs - output port list
    */
   template < class T   /* peek range obj,  */,
              typename std::enable_if<
                       ! std::is_base_of< autoreleasebase,
                                        T >::value >::type* = nullptr >
      bool send( T &range, Port &outputs )
   {
      auto * const fifo( select_fifo( outputs, sendtype ) );
      if( fifo != nullptr )
      {
         const auto space_avail(
            std::min( fifo->space_avail(), range.size() ) );

         using index_type = std::remove_const_t<decltype(space_avail)>;
         for( index_type i( 0 ); i < space_avail; i++ )
         {
            fifo->push( range[ i ].ele, range[ i ].sig );
         }
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
#endif /* END RAFTSPLITMETHOD_HPP */
