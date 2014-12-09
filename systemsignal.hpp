/**
 * systemsignal.hpp - definition header file for signal handler
 * function
 *
 * @author: Jonathan Beard
 * @version: Sat Dec  6 18:19:13 2014
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
#ifndef _SYSTEMSIGNAL_HPP_
#define _SYSTEMSIGNAL_HPP_  1

#include "signalvars.hpp"

namespace raft
{
   class kernel;
}

class FIFO;

/**
 * sighandler - takes a kernel pointer and a data struct that is
 * reserved for whatever future use we or whomever thinks of.  It
 * can always be void, but whatever signal handler you add, it must
 * be able to take the input.
 * @param port   - FIFO&, fifo that sent the signal 
 * @param kernel - raft::kernel* - kernel that fifo goes to 
 * @param signal - const raft::signal - curr signal thrown
 * @param data   - void ptr
 */
using sighandler = void (*)( FIFO               &fifo,
                             raft::kernel       *kernel, 
                             const raft::signal  signal,
                             void               *data ); 

#endif /* END _SYSTEMSIGNAL_HPP_ */
