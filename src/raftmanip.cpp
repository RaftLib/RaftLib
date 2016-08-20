/**
 * raftmanip.cpp - 
 * @author: Jonathan Beard
 * @version: Sat Aug 20 05:00:48 2016
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
#include "raftmanip.hpp"


const char* 
RaftManipException::what() const noexcept
{
    return( message.c_str() );
}

NonsenseChainRaftManipException::NonsenseChainRaftManipException(
    const raft::parallel::type a,
    const raft::parallel::type b
                                                                ) : RaftManipException( "{" + raft::parallel::type_name[ a ] + "} placed immediately before modifier " + raft::parallel::type_name[ b ] + "), which is nonsensical. Pleaes check your code and try again!" )
                                                                {}
