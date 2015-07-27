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
 main( int argc, char **argv )
 {
   int count( 1001 );
   if( argc == 2 )
   {
      count = atoi( argv[ 1 ] );
   }
   using gen   = raft::random_variate< std::int32_t, raft::sequential >;
   using p_gen = raft::print< std::int32_t  , '\n' >;
   raft::map.link(
      raft::kernel::make< gen >( 1, 1000, 1 , count ),
      raft::kernel::make< p_gen >() );
   raft::map.exe();
   return( EXIT_SUCCESS );
 }
