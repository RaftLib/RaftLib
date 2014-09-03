/**
 * portexception.cpp - 
 * @author: Jonathan Beard
 * @version: Wed Sep  3 14:52:27 2014
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
#include "portexception.hpp"

PortException::PortException( const std::string message )
{
   (this)->message = message;
}

const char*
PortException::what()
{
   return( message.c_str() );
}
