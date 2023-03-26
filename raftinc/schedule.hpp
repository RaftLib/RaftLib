/**
 * schedule.hpp -
 * @author: Jonathan Beard
 * @version: Thu Sep 11 15:42:28 2014
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
#ifndef RAFTSCHEDULE_HPP
#define RAFTSCHEDULE_HPP  1
#include <setjmp.h>
#include "signalvars.hpp"
#include "systemsignalhandler.hpp"
#include "rafttypes.hpp"
#include <set>
#include "kernelkeeper.tcc"
#include "mapbase.hpp"
#include "defs.hpp"
#include <affinity>

namespace raft {
    class kernel;
}

class Schedule
{
public:

    /**
     * Schedule - base constructor takes a map object
     * so that all sub-classes can access some of the
     * map features through the schedule sub-class
     * accessors.
     * @param   map - Map&
     */
    Schedule( MapBase &map ) : kernel_set( map.all_kernels ),
                               source_kernels( map.source_kernels ),
                               dst_kernels( map.dst_kernels ),
                               internally_created_kernels(
                                       map.internally_created_kernels )
    {
        //TODO, see if we want to keep this
        handlers.addHandler( raft::quit, Schedule::quitHandler );
    }

    /**
     * destructor, takes care of cleanup
     */
    virtual ~Schedule() = default;

    /**
     * start - called to start execution of all
     * kernels.  Implementation specific so it
     * is purely virtual.
     */
    virtual void start() = 0;


    /**
     * init - call to pre-process all kernels, this function
     * is called by the map object befure calling start.
     */
    virtual void init()
    {
        /** default, do nothing **/
    }

    /**
     * kernelRun - all the logic necessary to run a single
     * kernel successfully.  Any additional signal handling
     * should be handled by this function as its the only
     * one that will be universally called by the scheduler.
     * @param   kernel - raft::kernel *const object, non-null kernel
     * @param   finished - volatile bool - function sets to
     * true when done.
     * @return  true if run with no need for jmp_buf, false if
     * the scheduler needs to run again with the kernel_state
     */
    static bool kernelRun( raft::kernel * const kernel,
                           volatile bool &finished )
    {

        bool valid_run = false;
        if( kernelHasInputData( kernel ) )
        {
            const auto sig_status( kernel->run() );
            if( sig_status == raft::stop )
            {
                invalidateOutputPorts( kernel );
                finished = true;
            }
            valid_run = true;
        }
        /**
         * must recheck data items again after port valid check, there could
         * have been a push between these two conditional statements.
         */
        if( kernelHasNoInputPorts( kernel ) && ! kernelHasInputData( kernel ) )
        {
            invalidateOutputPorts( kernel );
            finished = true;
        }
        return valid_run;
    }

    //TODO, get rid of jmp_buf, no longer needed
    /**
     * scheduleKernel - adds the kernel "kernel" to the
     * schedule, ensures that it is run.  Other than
     * that there are no guarantees for its execution.
     * The base version should do for most, however feel
     * free to re-implement in derived class as long as
     * the source_kernels has all of the source kernels,
     * dst_kernels has all of the destination kernels, and
     * kernel_set has all of the kernels. Before
     * you drop in a kernel, it better be ready to go..all
     * allocations should be complete.
     * @param kernel - raft::kernel*
     */
    virtual void scheduleKernel( raft::kernel * const kernel )
    {
        /**
         * NOTE: The kernel param should be ready to rock,
         * we just need to add it here.  The data structures
         * xx_kernels all expect a fully ready kernel, well
         * the threads monitoring the system do at least and
         * this is one.  What we need to do to kick off the
         * execution is add it to the handleSchedule virtual
         * function which is implemented by each scheduler.
         */
        assert( kernel != nullptr );
        if( ! kernel->input.hasPorts() )
        {
            source_kernels += kernel;
        }
        if( ! kernel->output.hasPorts() )
        {
            dst_kernels += kernel;
        }
        kernel_set += kernel;
        if( kernel->internal_alloc )
        {
            internally_created_kernels += kernel;
        }
        handleSchedule( kernel );
        return;
    }
protected:
    virtual void handleSchedule( raft::kernel * const kernel ) = 0;
    /**
     * checkSystemSignal - check the incomming streams for
     * the param kernel for any system signals, if there
     * is one then consume the signal and perform the
     * appropriate action.
     * @param kernel - raft::kernel
     * @param data   - void*, use this if any further info
     *  is needed in future implementations of handlers
     * @return  raft::kstatus, proceed unless a stop signal is received
     */
    static raft::kstatus checkSystemSignal( raft::kernel * const kernel,
                                            void *data,
                                            SystemSignalHandler &handlers )
    {
        auto &input_ports( kernel->input );
        raft::kstatus ret_signal( raft::proceed );
        for( auto &port : input_ports )
        {
            if( port.size() == 0 )
            {
                continue;
            }
            const auto curr_signal( port.signal_peek() );
            if( R_UNLIKELY(
                ( curr_signal > 0 &&
                  curr_signal < raft::MAX_SYSTEM_SIGNAL ) ) )
            {
                port.signal_pop();
                /**
                 * TODO, right now there is special behavior for term signal
                 * only, what should we do with others?  Need to decide that.
                 */

                if( handlers.callHandler( curr_signal,
                                          port,
                                          kernel,
                                          data ) == raft::stop )
                {
                    ret_signal = raft::stop;
                }
            }
        }
        return( ret_signal );
    }

    /**
     * quiteHandler - performs the actions needed when
     * a port sends a quite signal (normal termination),
     * this is most likely due to the end of data.
     * @param fifo - FIFO& that sent the signal
     * @param kernel - raft::kernel*
     * @param signal - raft::signal
     * @param data   - void*, vain attempt to future proof
     */
    static raft::kstatus quitHandler( FIFO &fifo,
                                      raft::kernel *kernel,
                                      const raft::signal signal,
                                      void *data )
    {
        /**
         * NOTE: This should be the only action needed
         * currently, however that may change in the future
         * with more features and systems added.
         */
        UNUSED( kernel );
        UNUSED( signal );
        UNUSED( data );

        fifo.invalidate();
        return( raft::stop );
    }


    static void invalidateOutputPorts( raft::kernel *kernel )
    {

        auto &output_ports( kernel->output );
        for( auto &port : output_ports )
        {
            port.invalidate();
        }
        return;
    }

    /**
     * kernelHasInputData - check each input port for available
     * data, returns true if any of the input ports has available
     * data.
     * @param kernel - raft::kernel
     * @return bool  - true if input data available.
     */
    static bool kernelHasInputData( raft::kernel *kernel )
    {
        auto &port_list( kernel->input );
        if( ! port_list.hasPorts() )
        {
           /** only output ports, keep calling till exits **/
           return( true );
        }
        /**
         * NOTE: this was added as a reqeuest, need to update wiki,
         * the first hit to this one will take an extra few cycles
         * to process the jmp, however, after that, the branch
         * taken is incredibly easy and we should be able to do
         * this as if the switch statement wasn't there at all.
         * - an alternative to using the kernel variable would
         * be to implement a new subclass of kernel...that's doable
         * too but we'd have to make dependent template functions
         * that would use the type info to select the right behavior
         * which we're doing dynamically below in the switch statement.
         */
        switch( kernel->sched_behav )
        {
            case( raft::any_port ):
            {
                for( auto &port : port_list )
                {
                   const auto size( port.size() );
                   if( size > 0 )
                   {
                      return( true );
                   }
                }
            }
            break;
            case( raft::all_port ):
            {
                for( auto &port : port_list )
                {
                   const auto size( port.size() );
                   /** no data avail on this port, return false **/
                   if( size == 0 )
                   {
                      return( false );
                   }
                }
                /** all ports have data, return true **/
                return( true );
            }
            break;
            default:
            {
                //TODO add exception class here
                std::cerr << "invalid scheduling behavior set, exiting!\n";
                exit( EXIT_FAILURE );
            }
        }
        /** we should have returned before here, keep compiler happy **/
        return( false );
    }

    /**
     * kernelHasNoInputPorts - pretty much exactly like the
     * function name says, if the param kernel has no valid
     * input ports (this function assumes that kernelHasInputData()
     * has been called and returns false before this function
     * is called) then it returns true.
     * @params   kernel - raft::kernel*
     * @return  bool   - true if no valid input ports avail
     */
    static bool kernelHasNoInputPorts( raft::kernel *kernel )
    {
        auto &port_list( kernel->input );
        /** assume data check is already complete **/
        for( auto &port : port_list )
        {
            if( ! port.is_invalid() )
            {
                return( false );
            }
        }
        return( true );
    }


    /**
     * setPtrSets - add the tracking object from the
     * sub-class to track input/output ptrs in flight
     * by the kernel. The structure will be the same
     * across all in and output FIFOs, since from within
     * a "kernel" each fifo is accessed with sequential
     * consistency we won't need any fancy locking structures.
     * on each kernel firing, these structures should be
     * checked to see if any allocated objects (the only
     * ones to be put in these sets, need to be "garbage
     * collected"
     * @param kernel - raft::kernel* the one we're registering
     * @param in     - set_t*, the input set
     * @param out    - set_t*, the output set
     * @return void
     */
    static void setPtrSets( raft::kernel * const kernel,
                            ptr_map_t * const in,
                            ptr_set_t * const out,
                            ptr_set_t * const peekset )
    {
        assert( in  != nullptr );
        assert( out != nullptr );
        assert( peekset != nullptr );
        /**
         * looks a bit odd initially, but the same
         * peekset is set for each kernel's FIFO's
         * within the view of the kernel they'll be
         * accessed sequentially so no contention
         */
        for( auto &port : kernel->input )
        {
            port.setPtrMap( in );
            port.setInPeekSet( peekset );
        }
        for( auto &port : kernel->output )
        {
            port.setPtrSet( out );
            port.setOutPeekSet( peekset );
        }
        return;
    }


    static void fifo_gc( ptr_map_t * const in,
                         ptr_set_t * const out,
                         ptr_set_t * const peekset )
    {
        auto in_begin( in->begin() ), in_end( in->end() );
        auto out_beg( out->begin() ), out_end( out->end() );
        while( out_beg != out_end )
        {
            if( (*out_beg) != (*in_begin).first )
            {
                void * const ptr = reinterpret_cast< void* >(
                        (*in_begin).first );
                (*in_begin).second( ptr );
                ++in_begin;
            }
            else
            {
                ++out_beg;
                ++in_begin;
            }
        }
        while( in_begin != in_end )
        {
                void * const ptr = reinterpret_cast< void* >(
                        (*in_begin).first );
                (*in_begin).second( ptr );
                ++in_begin;
        }
        in->clear();
        out->clear();
        peekset->clear();
        return;
    }
    /**
     * signal handlers
     */
    SystemSignalHandler handlers;

    /** kernel set **/
    kernelkeeper &kernel_set;
    kernelkeeper &source_kernels;
    kernelkeeper &dst_kernels;
    kernelkeeper &internally_created_kernels;
};
#endif /* END RAFTSCHEDULE_HPP */
