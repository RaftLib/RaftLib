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
   

   template< class scheduler = pool_schedule, 
             class allocator = dynalloc > 
      void exe()
   {
      checkEdges( source_kernels );
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
      /** join scheduler first **/
      sched_thread.join();
      /** scheduler done, cleanup alloc **/
      exit_alloc = true;
      mem_thread.join();
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
   void checkEdges( std::set< raft::kernel* > &source_k );
   
   /**
    * printEdges - print a nice pretty picture using graphviz
    * of the current layout, future versions will pop up a 
    * window and display the topology as a 3-d graph, but we'll
    * save that till everything is relatively stable.
    * @param source_k, &std::set< raft::kernel* >
    */
   void printEdges( std::set< raft::kernel* > &source_k );
   


   friend class Schedule;
   friend class Allocate;
};

#endif /* END _MAP_HPP_ */
