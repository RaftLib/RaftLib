/**
 * partition.hpp - 
 * @author: Jonathan Beard
 * @version: Tue Mar 10 13:23:12 2015
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
#ifndef _PARTITION_HPP_
#define _PARTITION_HPP_  1
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstdint>
/** TODO, might bring this lib into RaftLib **/
#include <scotch.h>
#include "graph.tcc"
#include "graphtools.hpp"

class Partition
{
public:
   Partition() = delete;

   template < class Container >
   static 
   void  
   simple( Container &c, 
           std::vector< std::size_t > &mapping,
           const std::size_t cores )
   {
      if( c.size() <= cores )
      {
         const auto length( c.size() );
         for( auto i( 0 ); i < length; i++ )
         {
            mapping.push_back( i );
         }
         return;
      }
      using nummap_t = std::map< raft::kernel*,
                                 std::size_t >;
      using raftgraph_t = raft::graph< std::int32_t,
                                       std::int32_t >;
      raftgraph_t raft_graph;
      /** get an ordering, can be optimized a bit further **/
      nummap_t numbering;
      {
         auto index( 0 );
         for( raft::kernel *k : c )
         {
            numbering.insert( std::make_pair( k, index++ ) );
         }
      }
      struct LocalData
      {
         LocalData( raftgraph_t &g, nummap_t &m )
                     : graph( g ),
                       num_map( m )
         {}

         raftgraph_t    &graph;
         nummap_t       &num_map;
      } d ( raft_graph, numbering );
      auto graph_function = 
         [&]( PortInfo &a,
              PortInfo &b,
              void *data )
      {
         auto *local_data(
            reinterpret_cast< LocalData* >( data ) );
         const auto num_src( local_data->num_map[ a.my_kernel ] );
         const auto num_dst( local_data->num_map[ b.my_kernel ] );
         local_data->graph.addEdge( num_src, num_dst, 1 ); 
      };
      GraphTools::BFS( c, 
                       graph_function,
                       (void*) &d, 
                       false );
      SCOTCH_Graph graph;
      if( SCOTCH_graphInit( &graph ) != 0 )
      {
         /** TODO, add RaftLib Exception **/
         std::cerr << "Failed to initialize graph!!\n";
         exit( EXIT_FAILURE );
      }
      auto table( raft_graph.getScotchTables() );
      if( SCOTCH_graphBuild( 
            &graph                  /** graph ptr     **/,
            0                       /** base value    **/,
            table.num_vertices      /** vertex nmbr (zero indexed)   **/,
            table.vtable            /** vertex tab **/,
            &table.vtable[ 1 ]      /** vendtab **/,
            nullptr           /** velotab **/,
            nullptr           /** vlbltab **/,
            table.num_edges                 /** edge number **/,
            table.etable             /** edge tab **/,
            table.eweight         /** edlotab **/
          ) != 0 )
      {
         /** TODO, add RaftLib Exception **/
         std::cerr << "Failed to build graph\n";
         exit( EXIT_FAILURE );
      }
      if( SCOTCH_graphCheck( &graph ) != 0 )
      {
         /** TODO, add RaftLib Exception **/
         std::cerr << "Graph is inconsistent\n";
         exit( EXIT_FAILURE );
      }
      /** TODO, we can do much more with this arch file **/
      SCOTCH_Arch archdat;
      if( SCOTCH_archInit( &archdat )  != 0 )
      {
         /** TODO, add RaftLib Exception **/
         std::cerr << "Architecture initialization failed\n";
         exit( EXIT_FAILURE );
      }
      /** core are equal **/
      if( SCOTCH_archCmplt( &archdat, cores /** num cores **/) != 0 )
      {
         /** TODO, add RaftLib Exception **/
         std::cerr << "Failed to create architecture file\n";
         exit( EXIT_FAILURE );
      }
      /** strategy **/
      SCOTCH_Strat stradat;
      if( SCOTCH_stratInit( &stradat ) != 0 )
      {
         /** TODO, add RaftLib Exception **/
         std::cerr << "Failed to init strategy!!\n";
         exit( EXIT_FAILURE );
      }
      /** build recursive strategy **/
      if( SCOTCH_stratGraphClusterBuild(
                                   &stradat,
                                   SCOTCH_STRATRECURSIVE,
                                   cores,
                                   .7,
                                   .0001) != 0 )
      {
         /** TODO, add RaftLib Exception **/
         std::cerr << "Failed to map strategy graph!!\n";
         exit( EXIT_FAILURE );
      }
      if( SCOTCH_graphMap( 
            &graph             /** graph ptr **/,
            &archdat,
            &stradat,
            table.partition    /** parttab **/
            ) != 0 )
      {
         /** TODO, add RaftLib Exception **/
         std::cerr << "Failed to map!!\n";
         exit( EXIT_FAILURE );
      }
      /** copy mapping **/ 
      for( SCOTCH_Num i( 0 ); i < table.num_vertices; i++ )
      {
         mapping.push_back( table.partition[ i ] );
      }
      /** call exit graph **/
      SCOTCH_graphExit( &graph    );
      SCOTCH_stratExit( &stradat );
      SCOTCH_archExit ( &archdat );
      return;
   }
};
#endif /* END _PARTITION_HPP_ */
