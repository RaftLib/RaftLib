/**
 * signaldata.cpp - 
 * @author: Jonathan Beard
 * @version: Sat Jan 10 20:29:38 2015
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
#include <cassert>
#include "signaldata.hpp"
#include "defs.hpp"

/** TODO: come back here and remodel, and well...make these functions do something **/
/** NOTE: see notes in header file **/
raft::signal
SignalData::get_signal(  const std::thread::id  id )
{
    UNUSED( id );
    assert( false );
    return( static_cast< raft::signal >( 0 ) ); 
}

void
SignalData::set_signal( const std::thread::id  id, 
                        const raft::signal  signal )
{
   UNUSED( id );
   UNUSED( signal );
}
