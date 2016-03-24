/**
 * prefetchEffective.cpp - 
 * @author: Jonathan Beard
 * @version: Thu Mar 10 09:36:13 2016
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

#include "prefetch.hpp"
#include <iostream>
#include <cstdlib>
#include <cstdint>
#include <random>
#include <chrono>

volatile std::int64_t *ptr;

int
main( int argc, char **argv )
{
    static const auto SIZE( 1000000 );
    auto *arr( new std::int64_t[ SIZE ] );
    std::vector< int > index;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis( 0 , SIZE );
    
    for( auto i( 0 ); i < SIZE; i++ )
    {
        index.emplace_back( dis(gen) );
        //index.emplace_back( i );
    }
    
    for( auto count( 0 ); count < 20; count++ )
    {
        const auto start = std::chrono::high_resolution_clock::now();
         
        for( auto i( 0 ); i < SIZE - 3 ; i++ )
        {
            raft::prefetch< raft::WRITE, 
                            raft::LOTS, 
                      sizeof( std::int64_t ) >( &arr[ index[ i + 3 ] ] );    
            ptr = &arr[ index[ i ] ];
            arr[ index[ i ] ] = --(*ptr);
        }

        const auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> diff( end-start );
        std::cout << diff.count() << "\n";
    }
    delete[]( arr );
    return( EXIT_SUCCESS );
}
