/**
 * allpush.cpp - 
 * @author: Jonathan Beard
 * @version: Sun May 20 16:07:01 2018
 * 
 * Copyright 2018 Jonathan Beard
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
#include <cassert>
#include <iostream>
#include <cstdint>
#include <cstdlib>
#include <raft>
#include <raftio>
#include <cassert>
#include "generate.tcc"


template< typename A, typename B, typename C > class sum : public raft::kernel_all
{
public:
   sum() : raft::kernel_all()
   {
      input.addPort< A >( "input_a" );
      input.addPort< B >( "input_b" );
      output.addPort< C  >( "sum" );
   }
   
   virtual raft::kstatus run()
   {
      A a;
      B b;
      auto &port_a( input[ "input_a" ] );
      auto &port_b( input[ "input_b" ] );
      raft::signal  sig_a( raft::none  ), sig_b( raft::none );
      port_a.pop( a, &sig_a );
      port_b.pop( b, &sig_b );
      assert( sig_a == sig_b );
      C c( a + b );
      output[ "sum" ].push( c , sig_a );
      if( sig_b == raft::eof )
      {
         return( raft::stop );
      }
      return( raft::proceed );
   }

};


int
main( int argc, char **argv )
{
   int count( 1000 );
   if( argc == 2 )
   {
      count = atoi( argv[ 1 ] );
   }
   using type_t = std::int64_t;
   using gen = raft::test::generate< type_t >;
   using sum = sum< type_t, type_t, type_t >;
   using p_out = raft::print< type_t, '\n' >;
   gen a( count ), b( count );
   sum s;
   p_out print;

   raft::map m;
   m += a >> s[ "input_a" ];
   m += b >> s[ "input_b" ];
   m += s >> print;
   m.exe();
   
   return( EXIT_SUCCESS );
}
