/**
 * map.cpp - 
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
#include <sstream>
#include <map>
#include <sstream>
#include <vector>
#include <typeinfo>
#include <cstdio>
#include <cassert>
#include <string>
#include <cstdlib>
#include <cstring>

#include "common.hpp"
#include "map.hpp"
#include "graphtools.hpp"

Map::Map() : MapBase()
{
  
}

Map::~Map()
{
   for( raft::kernel *kern : all_kernels )
   {
      delete( kern );
   }
}

void
Map::checkEdges( std::set< raft::kernel* > &source_k )
{
   /**
    * NOTE: will throw an error that we're not catching here
    * if there are unconnected edges...this is something that
    * a user will have to fix.  Otherwise will return with no
    * errors.
    */
   GraphTools::BFS( source_k, 
                    []( PortInfo &a, PortInfo &b, void *data ){ return; },
                    nullptr,
                    true );
   return;
}

/**
   void insert( raft::kernel &a,  PortInfo &a_out, 
                raft::kernel &b,  PortInfo &b_in,
                raft::kernel &i,  PortInfo &i_in, PortInfo &i_out );
**/
void 
Map::enableDuplication( std::set< raft::kernel* > &source_k )
{
    /** don't have to do this but it makes it far more apparent where it comes from **/
    void * const kernel_ptr( reinterpret_cast< void* >( &all_kernels ) );
    using kernel_ptr_t = 
      typename std::remove_reference< decltype( all_kernels ) >::type;
    /** need to grab impl of Lengauer and Tarjan dominators, use for SESE **/
    /** in the interim, restrict to kernels that are simple to duplicate **/
    GraphTools::BFS( source_k,
                     []( PortInfo &a, PortInfo &b, void *data )
                     {
                        auto * const all_k( reinterpret_cast< kernel_ptr_t* >( data ) );  
                        if( a.out_of_order && b.out_of_order )
                        {
                           /** case of inline kernel **/
                           if( b.my_kernel->input.count() == 1 and 
                               b.my_kernel->output.count() == 1 and 
                               a.my_kernel->dup_candidate  )
                           {
                              auto *kernel_a( a.my_kernel );
                              assert( kernel_a->input.count() == 1 );
                              auto &port_info_front( kernel_a->input.getPortInfo() );
                              auto *front( port_info_front.other_kernel );
                              auto &front_port_info( front->output.getPortInfo() );
                              /**
                               * front -> kernel_a goes to
                               * front -> split -> kernel_a 
                               */
                              auto *split( 
                                 static_cast< raft::kernel* >( 
                                    port_info_front.split_func() ) );
                              all_k->insert( split );
                              MapBase::insert( front,    front_port_info,
                                               kernel_a, port_info_front,
                                               split );

                              assert( kernel_a->output.count() == 1 );

                              /** 
                               * now we need the port info from the input 
                               * port on back 
                               **/

                              /**
                               * kernel_a -> back goes to
                               * kernel_a -> join -> back 
                               */
                              auto *join( static_cast< raft::kernel* >( a.join_func() ) );
                              all_k->insert( join );
                              MapBase::insert( a.my_kernel, a,
                                               b.my_kernel, b,
                                               join );
                              /** 
                               * finally set the flag to the scheduler
                               * so that the parallel map manager can
                               * pick it up an use it.
                               */
                              a.my_kernel->dup_enabled = true; 
                           }
                           /** parallalizable source, single output no inputs**/
                           else if( a.my_kernel->input.count() == 0 and
                                    a.my_kernel->output.count() == 1 )
                           {
                              auto *join( static_cast< raft::kernel* >( a.join_func() ) );
                              all_k->insert( join );
                              MapBase::insert( a.my_kernel, a,
                                               b.my_kernel, b,
                                               join );
                              a.my_kernel->dup_enabled = true; 
                           }
                           /** parallelizable sink, single input, no outputs **/
                           else if( b.my_kernel->input.count() == 1 and
                                    b.my_kernel->output.count() == 0 )
                           {
                              auto *split( 
                                 static_cast< raft::kernel* >( b.split_func() ) );
                              all_k->insert( split );
                              MapBase::insert( a.my_kernel, a,
                                               b.my_kernel, b,
                                               split );
                              b.my_kernel->dup_enabled = true;
                           }
                           /** 
                            * flag as candidate if the connecting
                            * kernel only has one input port.
                            */
                           else if( b.my_kernel->input.count() == 1 )
                           {
                              /** simply flag as a candidate **/
                              b.my_kernel->dup_candidate = true;
                           }
                              
                        }
                     },
                     kernel_ptr,
                     false );
}

void
Map::printEdges( std::set< raft::kernel* > &source_k )
{
   std::stringstream                 gviz_output;
   std::map< std::string /* kernel name */, 
             std::size_t /* num */ > gviz_node_map;
   std::vector< std::string >        gviz_edge_map;
   std::size_t                       gviz_node_index( 0 );
   gviz_output << "digraph G{\n";
   gviz_output << "size=\"10,10\";\n";
   GraphTools::BFS( source_k,
                    [&]( const PortInfo &a, const PortInfo &b, void *data )
                    {
                        const std::string name_a( common::printClassName( *(a.my_kernel) ) );
                        const auto ret_val( gviz_node_map.insert( std::make_pair( name_a, gviz_node_index ) ) );
                        if( ret_val.second /* new kernel */ )
                        {
                           gviz_node_index++;
                        }
                        const std::string name_b( common::printClassName( *(b.my_kernel) ) );
                        const auto ret_val_2( gviz_node_map.insert( std::make_pair( name_b, gviz_node_index ) ) );
                        if( ret_val_2.second /* new kernel */ )
                        {
                           gviz_node_index++;
                        }
                        std::stringstream ss;
                        ss << gviz_node_map[ name_a ] << "->" << gviz_node_map[ name_b ] << "[ label=\"" <<
                           a.my_name << " to " << a.other_name << "\" ]";
                        gviz_edge_map.push_back( ss.str() );
                    },
                    nullptr,
                    false );
   for( auto it( gviz_node_map.begin() ); it != gviz_node_map.end(); ++it )
   {
      gviz_output << (*it).second << "[label=\"" << (*it).first << "\"];\n";
   }
   for( std::string &str : gviz_edge_map )
   {
      gviz_output << str << ";\n";
   }
   gviz_output << "}";
   std::cout << gviz_output.str();
   FILE *pipe( popen( "dot -Teps -oImage.eps", "w" ));
   assert( pipe != nullptr );
   const auto buff_size( gviz_output.gcount() + 1 );
   char *buffer = (char*) malloc( sizeof( char ) * ( buff_size ) );
   buffer[ buff_size - 1 ] = '\0';
   std::memcpy( buffer, gviz_output.str().c_str(), buff_size - 1 );
   fwrite( buffer, sizeof( char ), buff_size, pipe );
   pclose( pipe );
   free( buffer );
}

