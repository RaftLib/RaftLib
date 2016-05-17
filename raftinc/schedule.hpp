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
#ifndef _SCHEDULE_HPP_
#define _SCHEDULE_HPP_  1
#include <setjmp.h>
#include "signalvars.hpp"
#include "systemsignalhandler.hpp"
#include "rafttypes.hpp"
#include <set>
#include "kernelkeeper.tcc"
#include "defs.hpp"

namespace raft {
   class kernel;
   class map;
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
   Schedule( raft::map &map );
   
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
   virtual void init();
   
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
                          volatile bool       &finished,
                          jmp_buf             *gotostate    = nullptr,
                          jmp_buf             *kernel_state = nullptr  );
   
   /**
    * scheduleKernel - adds the kernel "kernel" to the
    * schedule, ensures that it is run.  Other than
    * that there are no guarantees for its execution.
    * It is purely virtual in its implementation.  Before
    * you drop in a kernel, it better be ready to go..all
    * allocations should be complete.
    * @param kernel - raft::kernel*
    */
   void scheduleKernel( raft::kernel * const kernel );
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
                                           SystemSignalHandler &handlers );

   /**
    * quiteHandler - performs the actions needed when
    * a port sends a quite signal (normal termination),
    * this is most likely due to the end of data.
    * @param fifo - FIFO& that sent the signal
    * @param kernel - raft::kernel*
    * @param signal - raft::signal
    * @param data   - void*, vain attempt to future proof
    */
   static raft::kstatus quitHandler( FIFO              &fifo,
                                     raft::kernel      *kernel,
                                     const raft::signal signal,
                                     void              *data );
   
   
   static void invalidateOutputPorts( raft::kernel *kernel );

   /** 
    * kernelHasInputData - check each input port for available
    * data, returns true if any of the input ports has available
    * data.
    * @param kernel - raft::kernel
    * @return bool  - true if input data available.
    */
   static bool kernelHasInputData( raft::kernel *kernel );
   
   /**
    * kernelHasNoInputPorts - pretty much exactly like the 
    * function name says, if the param kernel has no valid
    * input ports (this function assumes that kernelHasInputData()
    * has been called and returns false before this function 
    * is called) then it returns true.
    * @params   kernel - raft::kernel*
    * @return  bool   - true if no valid input ports avail
    */
   static bool kernelHasNoInputPorts( raft::kernel *kernel );

   
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
                           ptr_map_t    * const in,
                           ptr_set_t    * const out,
                           ptr_set_t    * const peekset );

   
   static void fifo_gc( ptr_map_t * const in,
                        ptr_set_t * const out,
                        ptr_set_t * const peekset );
   /**
    * signal handlers
    */
   SystemSignalHandler handlers;
   
   /** kernel set **/
   kernelkeeper &kernel_set;
   kernelkeeper &source_kernels;      
   kernelkeeper &dst_kernels;
};
#endif /* END _SCHEDULE_HPP_ */
