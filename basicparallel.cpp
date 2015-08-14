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

basic_parallel::basic_parallel( Map &map, volatile bool &exit_para )
   : source_kernels( map.source_kernels ),
     all_kernels(    map.all_kernels ),
     exit_para( exit_para )
{
   /** nothing to do here, move along **/
}


void
basic_parallel::start()
{
#if 0
   using hash_t = std::uintptr_t;
   std::map< hash_t, stats > hashmap; 
   while( ! exit_para )
   {
      GraphTools::BFS(  source_kernels, 
                        [&hashmap]( raft::kernel *kernel, 
                                    void *data )
                        { 
                           if( kernel->dup_enabled )
                           {                           
                              /** start checking stats **/
                              auto hash( 
                                 reinterpret_cast< std::uintptr_t >( kernel ) );
                              /** input  stats **/
                              raft::streamingstat< float > in;
                              raft::streamingstat< float > out;
                              for( auto &input  : kernel->input )
                              {
                                 in.update( input.size() / input.capacity() );
                              }
                              /** output stats **/
                              for( auto &output : kernel->output )
                              {
                                 out.update( output.size() / output.capacity() );
                              }
                              /** apply criteria **/
                              if( ( kernel->input.count() == 0 or 
                                    in.mean< float >() > .5  )  and 
                                  ( out.mean< float >() < .5  or 
                                    kernel->output.count() == 0 ) )
                              {
                                 hashmap[ hash ].occ_in++;
                              }
                              if( hashmap[ hash ].occ_in > 3 )
                              {
                                 /** clone **/
                                 if( kernel->input.count() != 0 )
                                 {
                                    auto &port_in( kernel->input.getPortInfo() );
                                 }
                                 if( kernel->output.count() != 0 )
                                 {
                                    auto *ptr( kernel->clone() );
                                    auto &port_out( kernel->output.getPortInfo() );
                                    const auto portid( 
                                       port_out.other_kernel->addPort() );
                                    auto &newinport(
                                       port_out.other_kernel->input.getPortInfoFor( 
                                          std::to_string( portid ) ) );
                                    auto &newoutport( ptr->output.getPortInfo() );
                                    
                                    //FIXME come back here
                                 }
                                 hashmap[ hash ].occ_in = 0;
                              }
                           }
                        },
                        nullptr );
      std::chrono::microseconds dura( 100 );
      std::this_thread::sleep_for( dura );
   }
#endif
}
