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
   split( const std::size_t num_ports = 1 ) : parallel_k()
   {
      input.addPort< T >( "0" );

      using index_type = std::remove_const_t<decltype(num_ports)>;
      for( index_type it( 0 ); it < num_ports; it++ )
      {
         addPort();
      }
   }

   virtual ~split() = default;
   
   IMPL_CLONE();

   virtual raft::kstatus run()
   {
      auto &input_port( input[ "0" ] );
      const auto avail( input_port.size() );
      auto range( input_port.template peek_range< T >( avail ) );
      /** split funtion selects a fifo using the appropriate split method **/
      auto items_sent( split_func.send( range, output ) );
      input_port.recycle( items_sent );
      input_port.unpeek();
      return( raft::proceed );
   }

   virtual std::size_t  addPort()
   {
      return( (this)->addPortTo< T >( output ) );
   }

protected:
   virtual void lock()
   {
      lock_helper( input );
   }

   virtual void unlock()
   {
      unlock_helper( input );
   }
   method split_func;
};
} /** end namespace raft **/
#endif /* END _SPLIT_TCC_ */
