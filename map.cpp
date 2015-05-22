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
                    []( const PortInfo &a, const PortInfo &b, void *data ){ return; },
                    nullptr,
                    true );
   return;
}

void 
Map::enableDuplication( std::set< raft::kernel* > &source_k )
{

    GraphTools::BFS( source_k,
                     []( const PortInfo &a, const PortInfo &b, void *data )
                     {
                        if( a.out_of_order && b.out_of_order )
                        {
                            /** TODO, finish thought **/
                        }
                     },
                     nullptr,
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

