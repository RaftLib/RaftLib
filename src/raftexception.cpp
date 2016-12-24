/**
 * raftexception.cpp - 
 * @author: Jonathan Beard
 * @version: Fri Dec 23 13:59:44 2016
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

#include <utility>
#include <cstring>
#include "raftexception.hpp"

RaftException::RaftException( const std::string &message ) : 
    message( std::move( message ) )
{
}

RaftException::RaftException( const std::string &&message ) : 
    message( message  )
{
}

const char*
RaftException::what() const noexcept
{
   return( strdup( message.c_str() ) );
}
