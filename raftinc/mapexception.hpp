/**
 * mapexception.hpp - 
 * @author: Jonathan Beard
 * @version: Wed Sep  3 14:52:27 2014
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
#ifndef MAPEXCEPTION_HPP
#define MAPEXCEPTION_HPP  1
#include <string>
#include <utility>

#include "raftexception.hpp"



class MapException : public RaftException 
{
public:
    MapException(  const std::string message ); 
};


template < int N > class MapExceptionBase : public MapException 
{
public:
    MapExceptionBase(  const std::string message ) : 
        MapException(  message ){};
};


using InvalidTopologyOperationException
    = MapExceptionBase< 0 >;

#endif /* END MAPEXCEPTION_HPP */
