/**
 * random.cpp - 
 * @author: Jonathan Beard
 * @version: Mon Mar  2 14:00:14 2015
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

 #include <raft>
 #include <raftrandom>
 #include <cstdint>
 #include <iostream>
 #include <raftio>


 int
 main()
 {
   using namespace raft;
   using type_t = std::uint32_t;
   const static auto send_size( 10 );
   using gen = random_variate< std::default_random_engine,
                               std::uniform_int_distribution,
                               type_t >;
   using p_out = raft::print< type_t, '\n' >;
   using sub = raft::lambdak< type_t >;
   
   std::vector< type_t > output;
   
   const static auto min( 0 );
   const static auto max( 100 );
   gen g( send_size, min, max );
   
   p_out print;

   auto  l_sub( [&]( Port &input,
                     Port &output )
      {
         std::uint32_t a;
         input[ "0" ].pop( a );
         output[ "0" ].push( a - 10 );
         return( raft::proceed );
      } );

   sub s( 1, 1, l_sub );

   raft::map m;
   m += g >> s >> print;
   
   m.exe();

   return( EXIT_SUCCESS );
 }
