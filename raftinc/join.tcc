/**
 * join.tcc - 
 * @author: Jonathan Beard
 * @version: Tue Oct 28 12:51:49 2014
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
#ifndef _JOIN_TCC_
#define _JOIN_TCC_  1
#include <raft>
#include <cstddef>


namespace raft{
template < class T, class method = roundrobin  > class join : public raft::parallel_k
{
public:
   join( const std::size_t num_ports = 1 ) : parallel_k()
   {
      output.addPort< T >( "0" );
      for( auto it( 0 ); it < num_ports; it++ )
      {
         addPort();
      }
   }

   virtual ~join() = default;

   virtual raft::kstatus run()
   {
      /** multiple calls to allocate will return same reference **/
      auto &output_port( output[ "0" ] );
      T &mem( output_port.template allocate< T >() );
      raft::signal temp_signal;
      if( split_func.get( mem, temp_signal, input ) )
      {
         /** call push to release above allocated memory **/
         output_port.send( temp_signal );
      }
      else /** didn't use allocated mem, deallocate **/
      {
         output_port.deallocate();
      }
      return( raft::proceed );
   }
   
   virtual std::size_t  addPort()
   {
      return( (this)->addPortTo< T >( input ) );
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
}
#endif /* END _JOIN_TCC_ */
