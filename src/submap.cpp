/**
 * submap.cpp - 
 * @author: Jonathan Beard
 * @version: Sun Nov 30 06:12:23 2014
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
#include "submap.hpp"

raft::submap::submap() : MapBase()
{
   /** nothing really to do **/
}

raft::submap::~submap()
{
   /** nothing really to do **/
}
    
void 
raft::submap::addInputKernel( raft::kernel * const k )
{
    assert( k != nullptr );
    assert( k->input.hasPorts() );
    auto &input_container( k->input );
    for( auto it( input_container.begin() ); it != input_container.end(); ++it )
    {
        input.insert( std::make_pair( it.name(), k ) );
    }
    return;
}

void 
raft::submap::addOutputKernel( raft::kernel * const k )
{
    assert( k != nullptr );
    assert( k->output.hasPorts() );
    auto &output_container( k->output );
    for( auto it( output_container.begin() ); it != output_container.end(); ++it )
    {
        output.insert( std::make_pair( it.name(), k ) );
    }
    return;
}
