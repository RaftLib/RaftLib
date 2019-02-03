/**
 * generate.tcc - 
 * @author: Jonathan Beard
 * @version: Fri Dec  4 07:26:02 2015
 * 
 * Copyright 2015 Jonathan Beard
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

#ifndef _GENERATE_TCC_
#define _GENERATE_TCC_  1

#include <raft>
namespace raft
{

namespace test
{

template < typename T > class generate : public raft::kernel
{
public:
   generate( std::int64_t count = 1000 ) : raft::kernel(),
                                         count( count )
   {
      output.addPort< T >( "number_stream" );
   }
   
   
   IMPL_CLONE();

   virtual raft::kstatus run()
   {
      if( count-- > 1 )
      {
         auto &ref( output[ "number_stream" ].template allocate< T >() );
         ref = static_cast< T >( (this)->count );
         output[ "number_stream"].send();
         return( raft::proceed );
      }
      /** else **/
      auto &ref( output[ "number_stream" ].template allocate< T >() );
      ref = static_cast< T >( (this)->count );
      output[ "number_stream" ].send( raft::eof );
      return( raft::stop );
   }

private:
   std::int64_t count;
};

} //end namespace test

} //end namespace raft
#endif /* END _GENERATE_TCC_ */
