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
#if UT_FOUND
#include <ut>
#endif
#if QTHREAD_FOUND
#include <qthread/qthread.hpp>
#else
/** dummy **/
using aligned_t = std::uint64_t;
#endif
#include "schedule.hpp"
#include "internaldefs.hpp"

namespace raft
{
    class kernel;
}


class pool_schedule : public Schedule
{
    static void main_wrapper( void* arg )
    {
        pool_schedule *this_ptr = (pool_schedule*) arg;
        this_ptr->main_handler();
    }

    void main_handler() {
        auto &container( this->kernel_set.acquire() );
#if USE_UT
        waitgroup_init(&wg);
        waitgroup_add(&wg, dst_kernels.size());
#endif
        for( auto * const k : container )
        {
            (this)->handleSchedule( k );
        }
        /**
         * NOTE: can't quite get the sync object behavior to work
         * quite well enough for this application. Should theoretically
         * work according to the documentation here:
         * http://www.cs.sandia.gov/qthreads/man/qthread_spawn.html
         * however it seems that the wait segfaults. Adding on the
         * TODO list I'll implement a better mwait monitor vs. spinning
         * which is relatively bad.
         */
        this->kernel_set.release();
#if USE_UT
        waitgroup_wait(&wg);
#else
        do
        {
            std::chrono::milliseconds dura( 3 );
            std::this_thread::sleep_for( dura );
            bool all_finished = true;
            this->tail_mutex.lock();
            for( auto * const td : this->tail )
            {
                if( ! td->finished )
                {
                    all_finished = false;
                    break;
                }
            }
            this->tail_mutex.unlock();
            if( all_finished ) {
                break;
            }
        } while( true );
#endif // USE_UT
    }
public:
    /**
     * pool_schedule - constructor, takes a map object,
     * calling this will launch threads.  scheduler itself
     * is also run as a thread.
     * @param   map - raft::map&
     */
    pool_schedule( MapBase &map ) : Schedule( map )
    {
#if USE_QTHREAD
        const auto ret_val( qthread_initialize() );
        if( ret_val != 0 )
        {
            std::cerr << "failure to initialize qthreads runtime, exiting\n";
            exit( EXIT_FAILURE );
        }
#endif
        thread_data_pool.reserve( kernel_set.size() );
    }

    /**
     * destructor, deletes threads and cleans up container
     * objects.
     */
    virtual ~pool_schedule()
    {
#if USE_QTHREAD
        /** kill off the qthread structures **/
        qthread_finalize();
#endif
    }

    /**
     * start - call to start executing map, at this point
     * the mapper sould have checked the topology and
     * everything should be set up for running.
     */
    virtual void start()
    {
#ifdef USE_UT
        const auto ret_val( runtime_initialize( NULL ) );
        // with cfg_path set to NULL, libut would getenv("LIBUT_CFG")
        if ( ret_val != 0 )
        {
            std::cerr << "failure to initialize libut runtime, exiting\n";
            exit( EXIT_FAILURE );
        }
        runtime_start( main_wrapper, this );
#elif defined(USE_QTHREAD)
        main_handler();
#endif
        return;
    }

protected:
    /**
     * modified version of what is in the simple_schedule
     * since we don't really need some of the info. this
     * is passed to each kernel within the pool_run func
     */
    struct ALIGN( 64 ) thread_data
    {
#pragma pack( push, 1 )
        constexpr thread_data( raft::kernel * const k ) : k( k ){}

        inline void setCore( const core_id_t core ){ loc = core; };
        /** this is deleted elsewhere, do not delete here, bad things happen **/
        raft::kernel *k = nullptr;
#if USE_UT
        waitgroup_t *wg = nullptr;
#else
        bool finished = false;
#endif
        core_id_t loc = -1;
#pragma pack( pop )
    };

    /** BEGIN FUNCTIONS **/
    /**
     * handleSchedule - handle actions needed to schedule the
     * kernel. This is mean to make maintaining the code a bit
     * easier.
     * @param    kernel - kernel to schedule
     */
    virtual void handleSchedule( raft::kernel * const kernel )
    {
        auto *td( new thread_data( kernel ) );
        thread_data_mutex.lock();
        thread_data_pool.emplace_back( td );
        thread_data_mutex.unlock();
        if( ! kernel->output.hasPorts() ) /** has no outputs **/
        {
#if USE_UT
            td->wg = &wg;
#else
            std::lock_guard< std::mutex > tail_lock( tail_mutex );
            /** destination kernel **/
            tail.emplace_back( td );
#endif
        }
#if USE_QTHREAD
        qthread_spawn( pool_schedule::pool_run,
                       (void*) td,
                       0,
                       0,
                       0,
                       nullptr,
                       NO_SHEPHERD,
                       0 );
#elif USE_UT
        thread_t *th_ptr = rt::Spawn( [td](){ pool_schedule::pool_run(td); } );
        UNUSED( th_ptr );
#endif
        /** done **/
        return;
    }

    /**
     * pool_run - pass this to the qthreads to run them.
     */
    static aligned_t pool_run( void *data )
    {
       assert( data != nullptr );
       auto * const thread_d( reinterpret_cast< thread_data* >( data ) );
       ptr_map_t in;
       ptr_set_t out;
       ptr_set_t peekset;

       Schedule::setPtrSets( thread_d->k,
                             &in,
                             &out,
                             &peekset );
#if 0 //figure out pinning later
       if( thread_d->loc != -1 )
       {
          /** call does nothing if not available **/
          raft::affinity::set( thread_d->loc );
       }
       else
       {
#ifdef USE_PARTITION
           assert( false );
#endif
       }
#endif
       volatile bool done( false );
       std::uint8_t run_count( 0 );
#ifdef BENCHMARK
       const auto init_id( raft::kernel::initialized_count() );
       while( raft::kernel::initialized_count( 0 ) !=
              raft::kernel::kernel_count( 0 ) )
       {
           raft::yield();
       }
#endif
       while( ! done )
       {
           Schedule::kernelRun( thread_d->k, done );
           //FIXME: add back in SystemClock user space timer
           //set up one cache line per thread
           if( run_count++ == 20 || done )
           {
               run_count = 0;
               //takes care of peekset clearing too
               Schedule::fifo_gc( &in, &out, &peekset );
               raft::yield();
           }
       }
#if USE_UT
       if( thread_d->wg )
       {
           // only tail kernel has wg pointer assigned
           waitgroup_done( thread_d->wg );
       }
#else
       thread_d->finished = true;
#endif
       return( 1 );
    }

    std::mutex thread_data_mutex;
    std::vector< thread_data* > thread_data_pool;
#if USE_UT
    waitgroup_t wg;
#else
    std::mutex tail_mutex;
    std::vector< thread_data* > tail;
#endif
};
#endif /* END RAFTPOOLSSCHEDULE_HPP */
