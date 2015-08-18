/**
 * map.hpp - 
 * @author: Jonathan Beard
 * @version: Fri Sep 12 10:28:33 2014
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
#ifndef _MAP_HPP_
#define _MAP_HPP_  1
#include <typeinfo>
#include <cassert>
#include <cxxabi.h>
#include <thread>
#include <sstream>

#include "kernelkeeper.tcc"
#include "portexception.hpp"
#include "schedule.hpp"
#include "simpleschedule.hpp"
#include "kernel.hpp"
#include "port_info.hpp"
#include "allocate.hpp"
#include "dynalloc.hpp"
#include "stdalloc.hpp"
#include "mapbase.hpp"
#include "poolschedule.hpp"
#include "basicparallel.hpp"
#include "noparallel.hpp"
#include "Clock.hpp"
#include "SystemClock.tcc"

/** lives in clockimpl.cpp **/
extern Clock *system_clock;

class Map : public MapBase
{
public:
   /** 
    * Map - constructor, really doesn't do too much at the monent
    * and doesn't really need to.
    */
   Map();
   /** 
    * default destructor 
    */
   virtual ~Map();
   
   /** 
    * FIXME, the graph tools need to take more than
    * function, we're wasting time by traversing
    * the graph twice....will take awhile with big
    * graphs.
    */
   template< class scheduler           = simple_schedule, 
             class allocator           = stdalloc,
             class parallelism_monitor = basic_parallel > 
      void exe()
   {
      for( auto * const submap : sub_maps )
      {
         auto &container( all_kernels.acquire() );
         auto &subcontainer( submap->all_kernels.acquire() );  
         container.insert( subcontainer.begin(),
                           subcontainer.end()   );
         all_kernels.release();
         submap->all_kernels.release();
      }
      /** check types, ensure all are linked **/
      checkEdges( source_kernels );
      /** adds in split/join kernels **/
      enableDuplication( source_kernels, all_kernels );
      volatile bool exit_alloc( false );
      allocator alloc( (*this), exit_alloc );
      /** launch allocator in a thread **/
      std::thread mem_thread( [&](){
         alloc.run();
      });
     
      alloc.waitTillReady();

      scheduler sched( (*this) );
      sched.init();
      
      /** launch scheduler in thread **/
      std::thread sched_thread( [&](){
         sched.start();
      });

      volatile bool exit_para( false );
      /** launch parallelism monitor **/
      parallelism_monitor pm( (*this)     /** ref to this    **/, 
                              alloc       /** allocator      **/,
                              sched       /** scheduler      **/,
                              exit_para   /** exit parameter **/);
      std::thread parallel_mon( [&](){
         pm.start();
      });
      /** join scheduler first **/
      sched_thread.join();

      /** scheduler done, cleanup alloc **/
      exit_alloc = true;
      mem_thread.join();
      /** no more need to duplicate kernels **/
      exit_para = true;
      parallel_mon.join();

      /** all fifo's deallocated when alloc goes out of scope **/
      return; 
   }


protected:

   /**
    * checkEdges - runs a breadth first search through the graph
    * to look for disconnected edges.
    * @param   source_k - std::set< raft::kernel* >
    * @throws PortException - thrown if an unconnected edge is found.
    */
   void checkEdges( kernelkeeper &source_k );

   /**
    * enableDuplication - add split / join kernels where needed, 
    * for the moment we're going with a simple split/join topology,
    * however that doesn't mean that more complex topologies might
    * not be implemented in the future.
    * @param    source_k - std::set< raft::kernel* > with sources
    */
   void enableDuplication( kernelkeeper &source, kernelkeeper &all );


   /** 
    * TODO, refactor basic_parallel base class to match the
    * all caps base class coding style
    */
   friend class basic_parallel;
   friend class Schedule;
   friend class Allocate;

};

#endif /* END _MAP_HPP_ */
