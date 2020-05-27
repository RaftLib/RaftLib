/**
 * raftexception.cpp - 
 * @author: Jonathan Beard
 * @version: Fri Dec 23 13:59:44 2016
 * 
 * Copyright 2020 Jonathan Beard
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

#include <utility>
#include <cassert>
#include <cstring>
#include "raftexception.hpp"

RaftException::RaftException( const std::string message ) : 
    message( 
#if (defined _WIN64 ) || (defined _WIN32) 
    /**
     * fix for warning C4996: 'strdup': The POSIX name for this item is
     * a bit annoying given it is a POSIX function, but this is the best
     * we can do, see: https://bit.ly/2TH0Jci
     */
    _strdup( message.c_str() )
#else //everybody else
    strdup( message.c_str() ) 
#endif
    )
{
}

const char*
RaftException::what() const noexcept
{
    assert( message != nullptr );
    return( message );
}
