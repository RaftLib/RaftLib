/**
 * signal.hpp - 
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
#ifndef _SIGNAL_HPP_
#define _SIGNAL_HPP_  1
#include <cstddef>
#include "signalvars.hpp"

namespace Buffer
{
struct Signal
{
   Signal();
   Signal( const Signal &other );

   Signal& operator = ( raft::signal signal );
   Signal& operator = ( raft::signal &signal );

   operator raft::signal ();

   std::size_t getindex();

   raft::signal sig   = raft::none;
   std::size_t  index = 0;
};
} /** end namespace buffer **/
#endif /* END _SIGNAL_HPP_ */
