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
#include "signalvars.hpp"
#include "systemsignalhandler.hpp"

namespace raft {
   class kernel;
}
class Map;

class Schedule
{
public:
   
   Schedule( Map &map );
   virtual ~Schedule();
   
   /**
    * start - called to start execution of all
    * kernels.  Implementation specific so it
    * is purely virtual.
    */
   virtual void start() = 0;

  
   /** 
    * init - call to pre-process all kernels, this function
    * is called by the map object
    */
   virtual void init();
protected:
  
   /**
    * checkSystemSignal - check the incomming streams for
    * the param kernel for any system signals, if there 
    * is one then consume the signal and perform the 
    * appropriate action.
    * @param kernel - raft::kernel
    * @param data   - void*, use this if any further info
    *  is needed in future implementations of handlers
    */
   void checkSystemSignal( raft::kernel * const kernel, void *data );

   /**
    * termHandler - static term handler function, 
    * passes term signal through from the currently
    * scheduled kernel to all the subsequent kenrels
    */
   static void termHandler( const raft::signal signal,
                            raft::kernel      *kernel,
                            void              *data );
   
   /**
    * scheduleKernel - adds the kernel "kernel" to the
    * schedule, ensures that it is run.  Other than
    * that there are no guarantees for its execution.
    * It is purely virtual in its implementation.
    * @param kernel - raft::kernel*
    * @return  bool  - returns false if the kernel is
    * already scheduled.
    */
   virtual bool scheduleKernel( raft::kernel *kernel );

   static void sendEndOfData( raft::kernel *kernel,
                              void *data );
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
    * signal handlers
    */
   SystemSignfalHandler handlers;

private:
   Map &map_ref;
};
#endif /* END _SCHEDULE_HPP_ */
