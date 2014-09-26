/**
 * ringbufferbase.tcc - 
 * @author: Jonathan Beard
 * @version: Thu May 15 09:06:52 2014
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
#ifndef _RINGBUFFERBASE_TCC_
#define _RINGBUFFERBASE_TCC_  1

#include <array>
#include <cstdlib>
#include <cassert>
#include <thread>
#include <cstring>
#include <iostream>
#include <cstddef>

#include "Clock.hpp"
#include "pointer.hpp"
#include "ringbuffertypes.hpp"
#include "bufferdata.tcc"
#include "signalvars.hpp"
#include "blocked.hpp"
#include "fifo.hpp"
#include "fifoabstract.tcc"
/**
 * Note: there is a NICE define that can be uncommented
 * below if you want sched_yield called when waiting for
 * writes or blocking for space, otherwise blocking will
 * actively spin while waiting.
 */
#define NICE 1

extern Clock *system_clock;

/** heap implementation, uses thread shared memory or SHM **/
#include "ringbufferheap.tcc"

/** infinite dummy implementation, can use shared memory or SHM **/
#include "ringbufferinfinite.tcc"

#endif /* END _RINGBUFFERBASE_TCC_ */
