/**
 * signal.cpp - 
 * @author: Jonathan Beard
 * @version: Wed Dec 31 15:14:56 2014
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
#include "signal.hpp"

using namespace Buffer;

Signal::Signal()
{

}

Signal::Signal( const Signal &other )
{
   (this)->sig = other.sig;
}

Signal& 
Signal::operator = ( raft::signal signal )
{
   (this)->sig = signal;
   return( (*this) );
}

Signal& 
Signal::operator = ( raft::signal &signal )
{
   (this)->sig = signal;
   return( (*this) );
}

Signal::operator raft::signal ()
{
   return( (this)->sig );
}

std::size_t 
Signal::getindex()
{
   return( index );
}
