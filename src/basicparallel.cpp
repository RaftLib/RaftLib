/**
 * basicparallel.cpp -
 * @author: Jonathan Beard
 * @version: Mon Aug 10 20:00:25 2015
 *
 * Copyright 2015 Jonathan Beard
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
#include "basicparallel.hpp"
#include "map.hpp"
#include "common.hpp"
#include "streamingstat.tcc"
#include <map>
#include "defs.hpp"



basic_parallel::basic_parallel( raft::map &map,
                                Allocate &alloc,
                                Schedule &sched,
                                volatile bool &exit_para )
   : source_kernels( map.source_kernels ),
     all_kernels(    map.all_kernels ),
     alloc( alloc ),
     sched( sched ),
     exit_para( exit_para )
{
   /** nothing to do here, move along **/
}


void
basic_parallel::start()
{
   using hash_t = std::uintptr_t;
   //FIXME, need to add the code that'll limit this without a count
   while( ! exit_para )
   {
      kernelkeeper::value_type &kernels( source_kernels.acquire() );
      /**
       * since we have to have a lock on the ports
       * for both BFS and duplication, we'll mark
       * the kernels inside of BFS and duplicate
       * outside of it.
       */
      std::vector< raft::kernel* > dup_list;
      GraphTools::BFS(  kernels,
                        (vertex_func) [&dup_list]( raft::kernel *kernel,
                                           void *data )
                        {
                           static std::map< hash_t, stats > hashmap;
                           static uint64_t count( 0 );

                           UNUSED( data );

                           if( kernel->dup_enabled )
                           {
                              /** start checking stats **/
                              auto hash(
                                 reinterpret_cast< std::uintptr_t >( kernel ) );
                              /** input  stats **/
                              raft::streamingstat< float > in;
                              raft::streamingstat< float > out;
                              if( kernel->input.hasPorts() )
                              {
                                 auto &input( kernel->input.getPortInfo() );
                                 auto *fifo( input.getFIFO() );
                                 in.update( static_cast< float >( fifo->size() )/ 
                                                static_cast< float >( fifo->capacity() ) );
                              }
                              if( kernel->output.hasPorts() )
                              {
                                 auto &output( kernel->output.getPortInfo() );
                                 auto *fifo( output.getFIFO() );
                                 out.update( static_cast< float >( fifo->size() )/ 
                                                  static_cast< float >( fifo->capacity() ) );
                              }
                              /** apply criteria **/
                              if( ( kernel->input.count() == 0 ||
                                    in.mean< float >() > .5  ) &&
                                  ( out.mean< float >() < .5   ||
                                    kernel->output.count() == 0 ) )
                              {
                                 hashmap[ hash ].occ_in++;
                              }
                              if( hashmap[ hash ].occ_in > 3 && count < 2 )
                              {
                                 count++;
                                 dup_list.emplace_back( kernel );
                                 hashmap[ hash ].occ_in = 0;
                              }
                           }
                        },
                        nullptr );
      source_kernels.release();

      for( auto * kernel : dup_list )
      {
         /**
          * FIXME, logic below only works for
          * single input, single output..intended
          * to get it working
          */
         /** clone **/
         auto *ptr( kernel->clone() );
         /** attach ports **/
         if( kernel->input.count() != 0 )
         {
            auto &old_port_in( kernel->input.getPortInfo() );
            old_port_in.other_kernel->lock();
            const auto portid(
               old_port_in.other_kernel->addPort() );

            auto &new_other_outport(
               old_port_in.other_kernel->output.getPortInfoFor(
#if STRING_NAMES
                   std::to_string( portid )
#else
                   portid 
#endif                   
               )
            );
            auto &new_port_in( ptr->input.getPortInfo() );
            /**
             * connecting a.y -> b.x
             * new_other_outprt == port y on a
             * new_port_in      == port x on b
             */
            alloc.allocate( new_other_outport,
                            new_port_in,
                            nullptr );
            old_port_in.other_kernel->unlock();
         }
         if( kernel->output.count() != 0 )
         {
            auto &old_port_out( kernel->output.getPortInfo() );
            old_port_out.other_kernel->lock();
            const auto portid(
               old_port_out.other_kernel->addPort() );
            auto &new_other_inport(
               old_port_out.other_kernel->input.getPortInfoFor(
#if STRING_NAMES
                  std::to_string( portid )
#else
                  portid 
#endif
                  ) );

            auto &newoutport( ptr->output.getPortInfo() );
            /**
             * connecting b.y -> c.x
             * newoutport       == port y on b
             * new_other_inport == port x on c
             */
            alloc.allocate( newoutport,
                            new_other_inport,
                            nullptr );

            old_port_out.other_kernel->unlock();
         }
         /** schedule new kernel **/
         sched.scheduleKernel( ptr );
      }


      dup_list.clear();
      std::chrono::microseconds dura( 100 );
      std::this_thread::sleep_for( dura );
   }
   return;
}
