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
    * scheduleKernel - adds the kernel "kernel" to the
    * schedule, ensures that it is run.  Other than
    * that there are no guarantees for its execution.
    * It is purely virtual in its implementation.
    * @param kernel - raft::kernel*
    * @return  bool  - returns false if the kernel is
    * already scheduled.
    */
   virtual bool scheduleKernel( raft::kernel *kernel );

   /**
    * invalidateOutputPorts - send invalidation signal via
    * each output port of param kernel.  This function calls
    * the FIFO specific implementation of invalidate.
    * @param   kernel - raft::kernel*
    */
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
    * @param   kernel - raft::kernel*
    * @return  bool   - true if no valid input ports avail
    */
   static bool kernelHasNoInputPorts( raft::kernel *kernel );

private:
   Map &map_ref;
};
#endif /* END _SCHEDULE_HPP_ */
