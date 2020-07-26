/**
 * map.hpp - 
 * @author: Jonathan Beard
 * @version: Fri Sep 12 10:28:33 2014
 * 
 * Copyright 2017 Jonathan Beard
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
#ifndef RAFTMAP_HPP
#define RAFTMAP_HPP  1
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

namespace raft
{

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
   virtual ~map() override;
   
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
             
        if( ! (this)->is_initialized )
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
            /** check types, ensure all are linked **/
            checkEdges();
            partition pt;
            pt.partition( all_kernels );
            /** adds in split/join kernels **/
            //enableDuplication( source_kernels, all_kernels );
            alloc_object = new allocator( (*this), exit_alloc );
            /** launch allocator in a thread **/
            allocator_thread = new std::thread( [&](){
               alloc_object->run();
            });
            
            try
            {
              alloc_object->waitTillReady();
            }
            catch( std::exception &ex )
            {
              std::cerr << "Exception caught with (" << ex.what() << ")\n"; 
            }
            
            sched_object = new scheduler( (*this) );
            sched_object->init();
            
            /** launch scheduler in thread **/
            schedule_thread = new std::thread( [&](){
               sched_object->start();
            });


            /** launch parallelism monitor **/
            pm = new parallelism_monitor(  (*this)               /** ref to this    **/, 
                                           (*alloc_object)       /** allocator      **/,
                                           (*sched_object)       /** scheduler      **/,
                                           exit_para   /** exit parameter **/);

            pm_thread = new std::thread( [&](){
               pm->start();
            });
            (this)->is_initialized = true;
        }
        else
        {
            //reinitialize kernels
        }
        
        //treat as barrier

        return; 
   }

   /** 
    * invoked to add kernel links to the map, returns an iterator to
    * a list of the last kernels added to the map via the += 
    */
   kernel_pair_t operator +=( kpair &p );

   

protected:
    /** 
     * joink - convenience function for joining kernels from 
     * kpair objects
     * @param next - kpair* const, kpair created from linking process
     */
    void joink( kpair * const next );

    /**
     * checkEdges - runs a breadth first search through the graph
     * to look for disconnected edges.
     * @throws PortException - thrown if an unconnected edge is found.
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
      

    bool            is_initialized      = false;

    bool            exit_alloc          = false;
    bool            exit_para           = false;
    //FIXME - consider redoing alloc as an extension of std::thread
    Allocate        *alloc_object       = nullptr;
    std::thread     *allocator_thread   = nullptr;
    
    Schedule        *sched_object       = nullptr;
    std::thread     *schedule_thread    = nullptr;


    basic_parallel  *pm                 = nullptr;
    std::thread     *pm_thread          = nullptr;

private:
    using split_stack_t = std::stack< std::size_t >;
    using group_t = std::vector< raft::kernel* >;
    using up_group_t = std::unique_ptr< group_t >;
    using kernels_t = std::vector< up_group_t >;

    /**
     * inline_cont - takes care of >> syntax, even
     * multiple ones.
     * @param groups - list of groups, each group containing
     * multiple (or a single kernel)
     * @param   temp_groups - temporary storage space
     * for above
     * @param   next - kpair*, pair to be joined/split
     */
    void inline_cont ( kernels_t     &groups,
                       kernels_t     &temp_groups,
                       kpair * const next );

    /**
     * inline_split - takes care of the <= syntax 
     * so that you can embed multiple splits in a 
     * row, and still have the joins appear later
     * in the proper sequence.
     * @param split_stack - split_stack_t&, stack
     * for # of out edges in each split
     * @param groups - list of groups, each group containing
     * multiple (or a single kernel)
     * @param   temp_groups - temporary storage space
     * for above
     * @param   next - kpair*, pair to be joined/split
     */
    void inline_split( split_stack_t &split_stack,
                       kernels_t     &groups,
                       kernels_t     &temp_groups,
                       kpair * const next );

    void inline_join( split_stack_t &split_stack,
                      kernels_t &groups,
                      kernels_t &temp_groups,
                      kpair * const next );

    void inline_dup_join( kernels_t &groups,
                          kernels_t &temp_groups,
                          kpair * const next );

}; /** end map decl **/

} /** end namespace raft **/
#endif /* END RAFTMAP_HPP */
