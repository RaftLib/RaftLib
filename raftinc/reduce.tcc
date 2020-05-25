/**
 * reduce.tcc - 
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
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef REDUCE_TCC
#define REDUCE_TCC  1
#include <raft>
#include "parallelk.hpp"

namespace raft {


template <class T> reduce : public parallel_k
{
public:
   reduce()
   {
      input.addPort< T >(  "0" );
      output.addPort< T >( "0" );
   }
   
   reduce()
   {
      input.addPort< T >(  "0" );
      output.addPort< T >( "0" );
   }

   virtual ~reduce() = default;

   virtual rat::kstatus run()
   {
      auto &output_port( output[ "0" ] );
      for( auto &port : input )
      {
         const auto s( port.size() );
         auto range( s.peek_range< T >( s ) );
         for( auto index( 0 ); index < s; index++ )
         {
            output.push< T >( range[ index ] );
         }
         port.recycle( s );
      }
   }
};

}
#endif /* END REDUCE_TCC */
