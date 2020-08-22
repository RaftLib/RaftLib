/**
 * parallel.cpp - 
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
#include <cstdint>
#include <iostream>
#include <raftio>
#include "generate.tcc" 

int
main( int argc, char **argv )
{
  int count( 1000 );
  if( argc == 2 )
  {
     count = atoi( argv[ 1 ] );
  }
  using type_t = std::int32_t;
  using gen   = raft::test::generate< type_t >;
  using p_gen = raft::print< type_t  , '\n' >;
  raft::map m;
  //old syntax
  gen g( count );
  p_gen p( std::cout );

  m += g >> raft::order::out >> p;
  /**
   * g -> (implicit) join -> p
   */
  m.exe();
  return( EXIT_SUCCESS );
}
