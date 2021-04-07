/**
 * manipbase.cpp - 
 * @author: Jonathan Beard
 * @version: Wed Apr  7 06:40:24 2021
 * 
 * Copyright 2021 Jonathan Beard
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
#include "defs.hpp"
#include "manipbase.hpp"
    
bool 
raft::manipbase::invoke( raft::kernel &k )
{
    UNUSED( k );
    /** unimplemented **/
    return( false );
}

bool 
raft::mapbase::invoke( PortInfo &src, PortInfo &dst )
{
    UNUSED( src );
    UNUSED( dst );
    return( false );
}

raft::manipbase* 
raft::mapbase::get_next( raft::manip_base *m )
{
    /**
     * right now I don't think we really need to check m, 
     * we'll leave as an assert, however, let's re-evaluate
     * once we start using the interface in anger. 
     */
    assert( m != nullptr );
    /** 
     * we shouldn't check null here, the return value
     * will be checked by the caller.
     */
    return( m->next );
}
    

