/**
 * parsemap.cpp - 
 * @author: Jonathan Beard
 * @version: Sun Mar 19 05:04:05 2017
 * 
 * Copyright 2017 Jonathan Beard
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
#include "parsemap.hpp"

using namespace raft;

parsemap::parsemap() : submap()
{
   /** nothing really to do **/
}

parsemap::~parsemap()
{
    /** 
     * note: we could have arrived here in 
     * error state. TODO add var to indicate
     * if this destructor was called after an
     * error occured. Then we can clean up 
     * safely. Right now just assume we need
     * to clean up and get on with things.
     */
    while( ! state.empty() )
    {
        /** 
         * techically reference to pointer
         */
        auto *ptr( state.top() );
        delete( ptr );
        state.pop();
    }
    /** 
     * nothing else to clean up, rest...other virtual 
     * destructors.
     */
}

void
parsemap::push_state( raft::parse::state * const state )
{
    assert( state != nullptr ); 
#ifdef PARANOIA
    /** check to see if the state location has already been pushed **/
    assert( state != &state.top() );
#endif
    state.push( state );
}
