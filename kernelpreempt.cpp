/**
 * kernelpreempt.cpp - 
 * @author: Jonathan Beard
 * @version: Fri May 22 15:26:59 2015
 * 
 * Copyright 2015 Jonathan Beard
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
#include <cstdint>
#include <setjmp.h>

#include "kernelpreempt.hpp"
#include "kernel.hpp"

std::int32_t
kernel_preempt::setRunningState( raft::kernel * const k )
{
   return( static_cast< 
            std::int32_t >( setjmp( k->running_state ) ) );
}

std::int32_t
kernel_preempt::setPreemptState( raft::kernel * const k )
{
   return( static_cast< 
            std::int32_t >( setjmp( k->preempt_state ) ) );
}

void
kernel_preempt::preempt( raft::kernel * const k )
{
   longjmp( k->preempt_state , 1 );
}

void 
kernel_preempt::restore( raft::kernel * const k )
{
   longjmp( k->running_state, 1 );
}
