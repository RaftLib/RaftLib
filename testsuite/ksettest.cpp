/**
 * ksettest.cpp - test case for kset.tcc
 * @author: Jonathan Beard
 * @version: Sat Apr 16 05:58:21 2016
 * 
 * Copyright 2016 Jonathan Beard
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
#include <vector>
#include <iostream>
#include <cstdlib>
//so we can test the basic kset structure w/o invoking raft topo
#define TEST_WO_RAFT 1
#include "kset.tcc"


int main()
{
    int a( 1 );
    int b( 2 );
    int c( 3 );
    auto s( raft::kset( a, b, c ) );
    std::vector< int > output;
    for( const auto *val : s )
    {
       output.emplace_back( *val ); 
    }
    assert( output.size() == 3 );
    int count( 1 );
    for( const auto val : output )
    {
        if( val != count++ )
        {
            exit( EXIT_FAILURE );
        }
    }
    return( EXIT_SUCCESS );
}

#undef  TEST_WO_RAFT
