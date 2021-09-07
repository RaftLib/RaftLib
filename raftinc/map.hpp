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
/** includes all partitioners **/
#include "partitioners.hpp"
#include "streamparse.hpp"

namespace raft
{
/**
 * predeclaration, include in src
 */
class parsemap;

class map : public MapBase
{
public:
   /** 
    * map - constructor, really doesn't do too much at the monent
    * and doesn't really need to.
    */
   map();
   /** 
    * default destructor 
    */
   virtual ~map() = default;
   
   /** 
    * FIXME, the graph tools need to take more than
    * function, we're wasting time by traversing
    * the graph twice....will take awhile with big
    * graphs.
    */
   template< class partition           = 
/** 
 * a bit hacky, yes, but enables you to inject
 * the partitioner...if you'd like while keeping
 * the default selection somewhat optimal (i.e.,
 * enabling thread pinning if available by default)
 */
#ifdef __linux
#if USE_PARTITION
             partition_scotch
#else
             //partition_basic /** no scotch, simple affinity assign **/
             partition_dummy
#endif
#else /** OS X, WIN64 **/
             partition_dummy
#endif
, /** comma for above **/
             class scheduler           = 
#ifdef USEQTHREADS
             pool_schedule
#else
             simple_schedule
#endif             
             , 
             class allocator           = dynalloc,
             class parallelism_monitor = basic_parallel > 
   void exe()
   {
      {
         auto &container( all_kernels.acquire() );
         for( auto * const submap : sub_maps )
         {
            auto &subcontainer( submap->all_kernels.acquire() );  
            container.insert( subcontainer.begin(),
                              subcontainer.end()   );
            submap->all_kernels.release();
         }
         all_kernels.release();
      }
      /** check types, ensure all are linked **/
      checkEdges();
      partition pt;
      pt.partition( all_kernels );
      
      /** adds in split/join kernels **/
      //enableDuplication( source_kernels, all_kernels );
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

   /** 
    * invoked to add kernel links to the map, returns an iterator to
    * a list of the last kernels added to the map via the += 
    */
   kernel_pair_t operator +=( raft::parsemap_ptr pm );

   

protected:
    /** 
     * joink - convenience function for joining kernels from 
     * kpair objects
     * @param next - kpair* const, kpair created from linking process
     */
    void joink( kpair * const next );

   /**
    * checkEdges - runs a linear search through the vertices
    * to look for disconnected edges.
    * @throws PortUnconnectedException
    */
   void checkEdges();

   /**
    * enableDuplication - add split / join kernels where needed, 
    * for the moment we're going with a simple split/join topology,
    * however that doesn't mean that more complex topologies might
    * not be implemented in the future.
    * @param    source_k - std::set< raft::kernel* > with sources
    */
   void enableDuplication( kernelkeeper &source, 
                           kernelkeeper &all );


   /** 
    * TODO, refactor basic_parallel base class to match the
    * all caps base class coding style
    */
   friend class ::basic_parallel;
   friend class ::Schedule;
   friend class ::Allocate;

}; /** end map decl **/

} /** end namespace raft **/
#endif /* END _MAP_HPP_ */
