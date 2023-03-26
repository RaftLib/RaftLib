/*
 * partition_scotch.tcc -
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
#ifndef RAFTPARTITION_SCOTCH_TCC
#define RAFTPARTITION_SCOTCH_TCC  1
#ifdef USE_PARTITION
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <functional>
#include <thread>
#include "graph.tcc"
#include "port_info.hpp"
#include "interface_partition.hpp"

class partition_scotch : public interface_partition
{
using container_type = typename kernelkeeper::value_type;

public:
    partition_scotch() = default;

    virtual void partition( kernelkeeper &keeper )
    {
        const auto cores( std::thread::hardware_concurrency() );
        auto &c( keeper.acquire() );
        if( simple_check( c, cores ) )
        {
            keeper.release();
            return;
        }
        /** else use queue weights **/
        auto weight_func(
            []( PortInfo &a, PortInfo &b, void *weight_data ) -> weight_t
            {
                //FIXME -> lets use a memoization of prev. runs to do
                //initial partition in the future
                return( 1 );
            }
        );
        run_scotch( c,
                    cores,
                    weight_func,
                    nullptr );
        keeper.release();
        return;
    }

private:
    using weight_function_t =
       typename std::function< weight_t( PortInfo&,PortInfo&,void* ) >;

    using raftgraph_t = raft::graph< edge_id_t, weight_t >;

    void get_graph_info( container_type &c,
                         raftgraph_t &raft_graph,
                         weight_function_t weight_func,
                         void *weight_data )
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
            LocalData( raftgraph_t &g, nummap_t &m ) :
                graph( g ),
                num_map( m )
            {}

            raftgraph_t &graph;
            nummap_t &num_map;
        } d ( raft_graph, numbering );
        auto graph_function = [&]( PortInfo &a, PortInfo &b, void *data )
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

    void simple( container_type &c,
                 const core_id_t cores )
    {
        if( /** #kernels <= #cores **/ simple_check( c, cores ) )
        {
            return;
        }
        auto weight_func(
            []( PortInfo &a, PortInfo &b, void *weight_data ) -> weight_t
            {
                /** simple weight to start **/
                return( 1 );
            }
        );
        run_scotch( c,
                    cores,
                    weight_func,
                    nullptr );
        return;
    }

    /**
     * simple case check
     */
    bool simple_check( container_type &c,
                       const core_id_t cores )
    {
        if( c.size() <= cores )
        {
            core_id_t i( 0 );
            for( auto *kernel : c )
            {
                (this)->setCore( *kernel, i );
                i++;
            }
            return( true );
        }
        /** else do nothing, return false **/
        return( false );
    }

    /**
     * run scotch
     */
    void run_scotch( container_type &c,
                     const core_id_t cores,
                     weight_function_t weight_func,
                     void *weight );
    {
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

        auto *table( raft_graph.getScotchTables() );
        if( SCOTCH_graphBuild(
                    &graph /** graph ptr **/,
                    0 /** base value **/,
                    table->num_vertices /** vertex nmbr (zero indexed) **/,
                    table->vtable /** vertex tab **/,
                    &table->vtable[ 1 ] /** vendtab **/,
                    nullptr /** velotab **/,
                    nullptr /** vlbltab **/,
                    table->num_edges /** edge number **/,
                    table->etable /** edge tab **/,
                    table->eweight /** edlotab **/
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
            std::remove_reference< decltype( ( *table ) ) >::type::print(
                    std::cerr, ( *table ) );
            std::cerr << "\n";
            raft_graph.print( std::cerr );

            exit( EXIT_FAILURE );
        }
        /** TODO, we can do much more with this arch file **/
        SCOTCH_Arch archdat;
        if( SCOTCH_archInit( &archdat ) != 0 )
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
                    .01
            ) != 0 )
        {
            /** TODO, add RaftLib Exception **/
            std::cerr << "Failed to map strategy graph!!\n";
            exit( EXIT_FAILURE );
        }
        if( SCOTCH_graphMap(
                    &graph /** graph ptr **/,
                    &archdat,
                    &stradat,
                    table->partition /** parttab **/
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
         /** copy mapping **/
         auto it( c.begin() );
         for( auto i( 0 ); i < table->num_vertices; i++, ++it )
         {
             (this)->setCore( *( *it ), table->partition[ i ] );
         }
        /** call exit graph **/
        delete( table );
        SCOTCH_graphExit( &graph );
        SCOTCH_stratExit( &stradat );
        SCOTCH_archExit ( &archdat );
        return;
    }
};
#endif
#endif /* END _PARTITION_HPP_ */
