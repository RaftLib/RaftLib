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
#include <functional>
/** TODO, might bring this lib into RaftLib **/
#include <scotch.h>
#include "graph.tcc"
#include "graphtools.hpp"

class Partition
{

public:
   Partition() = delete;

   template < class Container,
              class MappingContainer >
   static 
   void  
   simple( Container         &c, 
           MappingContainer  &mapping,
           const std::size_t cores )
   {
      if( simple_check( c, mapping, cores ) )
      {
         return;
      }
      auto weight_func( 
         []( PortInfo &a, PortInfo &b, void *weight_data ) -> std::int32_t
         {
            /** simple weight to start **/
            return( 1 );
         }
      );
      run_scotch( c, 
                  mapping, 
                  cores, 
                  weight_func, 
                  nullptr );
      return;
   }

   template < class Container,
              class MappingContainer >
   static
   void 
   utilization_weighted( Container &c,
                         MappingContainer  &mapping,
                         const std::size_t cores )
   {
      if( simple_check( c, mapping, cores ) )
      {
         return;
      }
      /** else use queue weights **/
      auto weight_func(
         []( PortInfo &a, PortInfo &b, void *weight_data ) -> std::int32_t
         {
             const auto size( a.getFIFO()->size() );
             if( size > 0 )
             {
               return( size );
             }
             else
             {
               return( 1 );
             }
         }
      );
      run_scotch( c, 
                  mapping, 
                  cores, 
                  weight_func, 
                  nullptr );
      return;
   }


private:
   
   using weight_function_t = 
      typename std::function< std::int32_t( PortInfo&,PortInfo&,void* ) >;
   
   using raftgraph_t = raft::graph< std::int32_t,
                                    std::int32_t >;

   
   
   template < class Container >
      static
      void get_graph_info( Container         &c,
                           raftgraph_t       &raft_graph,
                           weight_function_t weight_func,
                           void              *weight_data )
   {
      using nummap_t = std::map< raft::kernel const *,
                                 std::size_t >;
      /** get an ordering, can be optimized a bit further **/
      nummap_t numbering;
      {
         auto index( 0 );
         for( raft::kernel const *k : c )
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
         //if( ! Schedule::isActive( a.my_kernel ) )
         //{
         //   return;
         //}
         auto * const local_data(
            reinterpret_cast< LocalData* >( data ) );
         const auto num_src( local_data->num_map[ a.my_kernel ] );
         const auto num_dst( local_data->num_map[ b.my_kernel ] );
         const auto weight( weight_func( a, b, weight_data ) );
         local_data->graph.addEdge( num_src, num_dst, weight ); 
         return;
      };
      GraphTools::BFS( c, 
                       graph_function,
                       (void*) &d, 
                       false );

   }
   
   /** 
    * simple case check
    */
   template < class Container,
              class MapContainer >
      static
      bool simple_check( Container &c, 
                         MapContainer &mapping,
                         const std::size_t cores )
   {
      if( c.size() <= cores )
      {
         const auto length( c.size() );
         for( auto i( 0 ); i < length; i++ )
         {
            mapping.emplace_back( i );
         }
         return( true );
      }
      /** else do nothing, return false **/
      return( false );
   }
   
   /**
    * run scotch 
    */
   template < class Container,
              class MapContainer >
      static
      void run_scotch( Container         &c,
                       MapContainer      &mapping,
                       const std::size_t cores,
                       weight_function_t weight_func,
                       void              *weight )
   {
#if 0
      static_assert( std::is_signed< 
         std::remove_reference< decltype( c.end() ) >::type >::value, 
            "Container must have signed types so that -1 may signify no mapping" );
#endif            
      raftgraph_t raft_graph;
      get_graph_info( c, 
                      raft_graph, 
                      weight_func, 
                      nullptr );
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
         std::remove_reference< decltype( table ) >::type::print( std::cerr, table );
         std::cerr << "\n";
         raft_graph.print( std::cerr );
         
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
#ifndef USE_HWLOC      
      /** core are equal **/
      if( SCOTCH_archCmplt( &archdat, cores /** num cores **/) != 0 )
      {
         /** TODO, add RaftLib Exception **/
         std::cerr << "Failed to create architecture file\n";
         exit( EXIT_FAILURE );
      }
#else
      /** core are equal **/
      if( SCOTCH_archCmpltw( &archdat, cores /** num cores **/) != 0 )
      {
         /** TODO, add RaftLib Exception **/
         std::cerr << "Failed to create architecture file\n";
         exit( EXIT_FAILURE );
      }
#endif
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
                                   SCOTCH_STRATSPEED,
                                   cores,
                                   .75,
                                   .01) != 0 )
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
      /**
       * first case is for if we've mapped all vertices, 
       * second is for when some of the kernels are innactive
       * in which case the number of vertices in the 
       * table will be less than the size of c in which case
       * we need to get which vertices (the actual number id
       * from the application) are mapped and to where, the 
       * returned table in mapping must include even the 
       * vertices that aren't active (indicated by a -1) so
       * that the returning loop can be as simple as possible
       */
      if( c.size() == table.num_vertices )
      {
         /** copy mapping **/ 
         for( auto i( 0 ); i < table.num_vertices; i++ )
         {
            mapping.emplace_back( table.partition[ i ] );
         }
      }
      else
      {
         const auto &vmapping( raft_graph.getVertexNumbersAtIndicies() );
         auto it_map_index( vmapping.cbegin() );
         auto table_index( 0 );
         const auto size( c.size() );
         for( auto i( 0 ); i < size; i++ )
         {
            if( i == (*it_map_index) &&  it_map_index != vmapping.cend() )
            {
               mapping.emplace_back( table.partition[ table_index++ ] );
               ++it_map_index;
            }
            else
            {
               mapping.emplace_back( -1 );
            }
         }
      }
      /** call exit graph **/
      SCOTCH_graphExit( &graph    );
      SCOTCH_stratExit( &stradat );
      SCOTCH_archExit ( &archdat );
      return;
   }

};
#endif /* END _PARTITION_HPP_ */
