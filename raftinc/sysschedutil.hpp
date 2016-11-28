/**
 * sysschedutil.hpp - 
 * @author: Jonathan Beard
 * @version: Sun Nov 27 09:11:32 2016
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
#ifndef _SYSSCHEDUTIL_HPP_
#define _SYSSCHEDUTIL_HPP_  1


#ifdef USEQTHREADS
#include <qthread/qthread.hpp>
#else
#include <sched.h>
#endif

namespace raft
{

static inline void yield()
{
#ifdef USEQTHREADS
    qthread_yield();
#else         
    sched_yield();
#endif
}

} /** end namespace raft **/

#endif /* END _SYSSCHEDUTIL_HPP_ */
