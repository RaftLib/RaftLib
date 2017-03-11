/**
 * allocate.hpp - allocate base class, extend me to build new
 * allocate classes.  This object has several useful features
 * and data structures, namely the set of all source kernels and
 * all the kernels within the graph.  There is also a list of 
 * all the currently allocated FIFO objects within the streaming
 * graph.  This is primarily for instrumentation puposes.
 * @author: Jonathan Beard
 * @version: Tue Sep 16 20:20:06 2014
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
#ifndef _ALLOCATE_HPP_
#define _ALLOCATE_HPP_  1

#include "kernelkeeper.tcc"
#include "kernel.hpp"
#include "port_info.hpp"
#include "fifo.hpp"
#include <set>

/**
 * ALLOC_ALIGN_WIDTH - there's probably a better way
 * to do this, but for the moment I'm mainly running
 * on x86_64 so this will work.  In the future I'l
 * see about defining others for other architectures
 * or perhaps start including the header files 
 * that reference the alignments.
 */

#if defined __AVX__ || __AVX2__ || _WIN64
#define ALLOC_ALIGN_WIDTH 32
#else
#define ALLOC_ALIGN_WIDTH 16
#endif

#define INITIAL_ALLOC_SIZE 64

namespace raft
{
    class map;
}

class basic_parallel;

class Allocate
{
public:
   /**
    * Allocate - base constructor, really doesn't do too much
    * save for setting the global variables all_kernels and 
    * source_kernels from the Map object.  
    * @param map - raft::map&
    * @param exit_alloc - bool used to terminate loop,
    *  for monitoring allocations, controlled by map 
    *  object.
    */
   Allocate( raft::map &map, volatile bool &exit_alloc );
   
   /**
    * destructor 
    */
   virtual ~Allocate();

   /**
    * run - implement this function to create a new allocator, will
    * be run inside a thread so exits when done but if run-time monitoring
    * is desired then this is the place to do it.
    */
   virtual void run() = 0;

   /** 
    * waitTillReady - call after initializing the allocate thread, returns
    * when the initial allocation is complete.
    */
   void waitTillReady() ;

   
protected:
   /**
    * initialize - internal method to be used within the run method
    * takes care of the initialization using the already allocated
    * FIFO object passed as a param.  This function will throw 
    * an exception if either port (src or dst) have already been
    * allocated.
    * @param   src - PortInfo*, nullptr if not to be set
    * @param   dst - PortInfo*, nullptr if not to be set
    * @param   fifo - FIFO*
    * @throws  PortDoubleInitializeException - if either port is already initialized.
    */
   void initialize( PortInfo * const src, 
                    PortInfo * const dst, 
                    FIFO * const fifo );

   
   virtual void allocate( PortInfo &a, PortInfo &b, void *data );

   /**
    * setReady - call within the implemented run function to signal
    * that the initial allocations have been completed.
    */
   void setReady() ;

   /** both convenience structs, hold exactly what the names say **/
   kernelkeeper   &source_kernels;
   kernelkeeper   &all_kernels;
   
   /** 
    * keeps a list of all currently allocated FIFO objects,
    * set from within the initialize function.
    */
   std::set< FIFO*   > allocated_fifo;

   /**
    * exit_alloc - bool whose value is set by the map 
    * object, controls when the loop within the alloc thread
    * is exited.
    */
   volatile bool &exit_alloc;
private:
   volatile bool ready = false;
   friend class basic_parallel;
};
#endif /* END _ALLOCATE_HPP_ */
