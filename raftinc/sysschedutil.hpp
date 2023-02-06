/**
 * sysschedutil.hpp - 
 * @author: Jonathan Beard
 * @version: Mon May 25 09:11:32 2020
 * 
 * Copyright 2020 Jonathan Beard
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
#ifndef RAFTSYSSCHEDUTIL_HPP
#define RAFTSYSSCHEDUTIL_HPP  1

#if (! defined _WIN64) && (! defined _WIN32)
#ifdef QTHREAD_FOUND
#include <qthread/qthread.hpp>
#endif
#ifdef UT_FOUND
#include <ut>
#endif
#include <sched.h>
#endif /** end if not win **/

namespace raft
{

/**
 * yield - generic yield function for whatever the underlying
 * implementation is, could be qthreads, a process, or a thread
 * but it'll call the right implementation for you. 
 */
static inline void yield()
{

#if (! defined _WIN64) && (! defined _WIN32)
#ifdef USE_UT
    rt::Yield();
#elif defined(USE_QTHREAD)
    qthread_yield();
#else
    sched_yield();
#endif
#endif /** end if not win **/
    return;
}

} /** end namespace raft **/

#endif /* END RAFTSYSSCHEDUTIL_HPP */
