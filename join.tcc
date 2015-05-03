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
#include "splitmethod.hpp"
#include "roundrobin.hpp"
#include "parallelk.hpp"

namespace raft{
template < class T, class method = roundrobin  > class join : public raft::parallel_k
{
public:
   join()
   {
      addPortTo< T >( input );
      output.addPort< T >( "0" );
   }

   virtual ~join() = default;

   virtual raft::kstatus run()
   {
      /** multiple calls to allocate will return same reference **/
      T &mem( output[ "0" ].template allocate< T >() );
      raft::signal temp_signal;
      if( split_func.get( mem, temp_signal, input ) )
      {
         /** call push to release above allocated memory **/
         output[ "0" ].send( temp_signal );
      }
      return( raft::proceed );
   }
   
   virtual void addPort()
   {
      addPortTo< T >( input );
   }

protected:

   method split_func; 
};
}
#endif /* END _JOIN_TCC_ */
