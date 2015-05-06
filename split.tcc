/**
 * split.tcc - 
 * @author: Jonathan Beard
 * @version: Mon Oct 20 09:49:17 2014
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
 * See the License for the specific language sendverning permissions and
 * limitations under the License.
 */
#ifndef _SPLIT_TCC_
#define _SPLIT_TCC_  1
#include <string>
#include <raft>

namespace raft {
template < class T, class method = roundrobin > class split : public raft::parallel_k
{
public:
   split()
   {
      input.addPort<  T >( "0" );
      addPortTo< T >( output );
   }

   virtual ~split() = default;

   virtual raft::kstatus run()
   {
      /** TODO, add code to do multi-item inserts **/
      raft::signal signal;
      auto &output_port( input[ "0" ] );
      T &item( output_port.template peek< T >( &signal ) );
      /** split funtion selects a fifo using the appropriate split method **/
      if( split_func.send( item, signal, output ) )
      {
         /* recycle item */
         output_port.recycle( 1 );
      }
      else
      {
         output_port.unpeek();
      }
      return( raft::proceed );
   }
   
   virtual void addPort()
   {
      addPortTo< T >( output );
   }

protected:
   method split_func;
};
} /** end namespace raft **/
#endif /* END _SPLIT_TCC_ */
