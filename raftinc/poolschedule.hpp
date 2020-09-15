/**
 * poolschedule.hpp - scheduler starts up the maximum number
 * of threads supported in hardware for the currently executing
 * node, then awaits kernels to be mapped to it by the mapper. 
 * Once mapped, the scheduler continues to execute each kernel
 * in a round-robin fashion until execution is complete.
 *
 * @author: Jonathan Beard
 * @version: Thu Sep 11 15:49:57 2014
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
#ifndef RAFTPOOLSSCHEDULE_HPP
#define RAFTPOOLSSCHEDULE_HPP  1
#include <vector>
#include <set>
#include <thread>
#include <mutex>
#include <cstdint>
#ifdef USEQTHREADS
#include <qthread/qthread.hpp>
#else
/** dummy **/
using aligned_t = std::uint64_t;
#endif
#include "schedule.hpp"
#include "internaldefs.hpp"

namespace raft{
   class kernel;
   class map;
}


class pool_schedule : public Schedule
{
public:
    /**
     * pool_schedule - constructor, takes a map object, 
     * calling this will launch threads.  scheduler itself
     * is also run as a thread.
     * @param   map - raft::map&
     */
    pool_schedule( raft::map &map );

    /**
     * destructor, deletes threads and cleans up container
     * objects.
     */
    virtual ~pool_schedule();

    /**
     * start - call to start executing map, at this point
     * the mapper sould have checked the topology and 
     * everything should be set up for running.
     */
    virtual void start(); 
   
protected:
    /** BEGIN FUNCTIONS **/
    /**
     * handleSchedule - handle actions needed to schedule the
     * kernel. This is mean to make maintaining the code a bit
     * easier.
     * @param    kernel - kernel to schedule
     */
    virtual void handleSchedule( raft::kernel * const kernel );
    /**
     * pool_run - pass this to the qthreads to run them.
     */
    static aligned_t pool_run( void *data );
   
    /** 
     * modified version of what is in the simple_schedule 
     * since we don't really need some of the info. this
     * is passed to each kernel within teh pool_run func
     */
    struct ALIGN( 64 ) thread_data
    {
#pragma pack( push, 1 )       
       constexpr thread_data( raft::kernel * const k ) : k( k ){}

       inline void setCore( const core_id_t core ){ loc = core; };
       /** this is deleted elsewhere, do not delete here, bad things happen **/
       raft::kernel *k         = nullptr;
       bool          finished  = false;
       core_id_t     loc       = -1;
#pragma pack( pop )       
    };
    std::mutex                  thread_data_mutex;
    std::vector< thread_data* > thread_data_pool;
    std::mutex                  tail_mutex;
    std::vector< thread_data* > tail;
};
#endif /* END RAFTPOOLSSCHEDULE_HPP */
