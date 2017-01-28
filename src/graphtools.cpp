/**
 * graphtools.cpp -
 * @author: Jonathan Beard
 * @version: Sat Sep 20 13:15:09 2014
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
#include <algorithm>
#include <set>
#include <map>
#include <queue>
#include <utility>
#include <string>
#include <sstream>
#include <mutex>

#include "portmap_t.hpp"
#include "common.hpp"
#include "portmap_t.hpp"
#include "portexception.hpp"
#include "graphtools.hpp"
#include "port_info.hpp"
#include "kernel.hpp"
#include "tempmap.hpp"
#include "utility.hpp"

void
GraphTools::BFT( std::set< raft::kernel* > &source_kernels,
                 edge_func func,
                 const graph_direction direction,
                 void      *data,
                 bool      connected_error )
{
   std::set< raft::kernel* >       visited_set;
   std::queue< raft::kernel* >     queue;
   std::for_each( source_kernels.begin(),
                  source_kernels.end(),
                  [&]( raft::kernel *k )
                  {
                     queue.push( k );
                     visited_set.insert( k );
                  } );
   GraphTools::__BFT(   queue, 
                        visited_set, 
                        func, 
                        direction, 
                        data, 
                        connected_error );
}

void
GraphTools::BFT( std::vector< raft::kernel* > &source_kernels,
                 edge_func func,
                 const graph_direction direction,
                 void      *data,
                 bool      connected_error )
{
   std::set< raft::kernel* >       visited_set;
   std::queue< raft::kernel* >     queue;
   std::for_each( source_kernels.begin(),
                  source_kernels.end(),
                  [&]( raft::kernel *k )
                  {
                     queue.push( k );
                     visited_set.insert( k );
                  } );
   GraphTools::__BFT(   queue, 
                        visited_set, 
                        func, 
                        direction, 
                        data, 
                        connected_error );
}


void
GraphTools::BFT( std::set< raft::kernel* > &source_kernels,
                 vertex_func                 func,
                 const graph_direction       direction,
                 void                        *data )
{
   std::queue< raft::kernel* >   queue;
   std::set< raft::kernel* >     visited_set;
   std::for_each( source_kernels.begin(),
                  source_kernels.end(),
                  [&]( raft::kernel * k )
                  {
                     queue.push( k );
                     visited_set.insert( k );
                  });

   GraphTools::__BFT(   queue, 
                        visited_set, 
                        func, 
                        direction, 
                        data );
   return;
}

raft::temp_map*
GraphTools::duplicateFromVertexToSource( raft::kernel * const start )
{
    assert( start != nullptr );
    struct Data
    {
        Data() : temp_map( new raft::temp_map() )
        {
        }
        /**
         * NOTE: this map contains a reference from the pointers
         * cast to uintptr_t (set as the key) that are the original 
         * (to be duplicated kernels) to the new one which is the 
         * value. 
         */
        std::map< std::uintptr_t, raft::kernel * >      kernel_map;
        /** 
         * for graphs with a feedback loop, back-edges, we could
         * in fact have an edge that appears in the BFT whose
         * other terminal end has yet to be reached.
         */
        std::map< std::uintptr_t, PortInfo* >           unmatched;
        raft::temp_map                                 *temp_map     = nullptr;
    }   d;

    auto updateUnmatched( []( Data * const d ) -> void 
    {
            auto intersect( raft::utility::intersect_map(
                d->unmatched,
                d->kernel_map
            ) );
            for( auto &match: (*intersect ) )
            {
                /** 
                 * second will be unmatched portinfo then the kernel map
                 * we need to take the portinfo object that now has an 
                 * initialized source and set up its destination. The name
                 * thats given is already correct, just not pointing to the
                 * correct (cloned) kernel.
                 */
                auto *portinfo( match.second.first );
                /** double check to make sure that source kernel hasn't been deallocated **/
                assert( portinfo->my_kernel != nullptr );
                /** sanity check on key type, make sure somebody didn't accidentally update destination kernel **/
                assert( reinterpret_cast< std::uintptr_t >( portinfo->other_kernel ) == match.first );
                d->temp_map->link(
                    portinfo->my_kernel     /** cloned source **/,
                    portinfo->my_name       /** output port name inherited from clone parent **/,
                    match.second.second         /** newly found cloned destination kernel        **/,
                    portinfo->other_name,
                    ( portinfo->out_of_order ? raft::order::out : raft::order::in ) );
                //TODO, add overkill asserts for each of these
                d->kernel_map.erase( match.first );
                d->unmatched.erase( match.first );
            }
    } );

    /** make structure to hold previously cloned kernel **/
    vertex_func f( [&updateUnmatched]( raft::kernel *current,
                       void         *data ) -> void
    {
        Data *d( reinterpret_cast< Data* >( data ) );
        auto *cloned_kernel( current->clone() );
        if( cloned_kernel == nullptr )
        {
            //TODO throw an exception
            std::cerr << 
                "attempting to clone a kernel that wasn't meant to be cloned()\n";
            exit( EXIT_FAILURE );
        }
        /** first kernel **/
        if( d->kernel_map.size() == 0 )
        {
            const auto d_ret_val( d->kernel_map.insert( std::make_pair( 
                reinterpret_cast< std::uintptr_t >( current ),
                cloned_kernel ) ) );
#if NDEBUG
            UNUSED( d_ret_val );
#else
            assert( d_ret_val.second == true );
#endif
            return;
        }
        else
        {
            
            const auto d_ret_val( d->kernel_map.insert( std::make_pair( 
                reinterpret_cast< std::uintptr_t >( current ),
                cloned_kernel ) ) );
#if NDEBUG
            UNUSED( d_ret_val );
#else
            assert( d_ret_val.second == true );
#endif
            /** 
             * loop over output ports of "current" kernel and hook the
             * corresponding ports of the cloned kernel to the corresponding
             * ports on the cloned output kernels in the map (inside data 
             * struct).
             */
            auto &map_of_ports( current->output.portmap.map );
            for( auto &port : map_of_ports )
            {   
                /** port.first is the name **/
                auto &port_info( port.second );
                /**
                 * we want to link clone.out to the 
                 * output kernel clone saved in the 
                 * map that corresponds to the memory
                 * location then link it to the new 
                 * kernels port.
                 */
                const auto dst_kern_hash( 
                    reinterpret_cast< std::uintptr_t >( 
                        port_info.other_kernel
                    )
                );
                /** 
                 * look up this destination kernel in our
                 * clone map.
                 */
                auto found( d->kernel_map.find( dst_kern_hash ) );
                if( found == d->kernel_map.end() )
                {
                    /** 
                     * get port info for the current named port, what we want is the cloned kernel's 
                     * port info then index it on the destination that we're waiting on, when the
                     * destination appears in the kernel_map then we can complete the link 
                     */
                    auto &cloned_port_info( cloned_kernel->output.getPortInfoFor( port_info.my_name ) );
                    /** destination not yet reached, likely back edge **/
                    d->unmatched.insert( 
                        std::make_pair( 
                            reinterpret_cast< std::uintptr_t >( dst_kern_hash ) /** kern we're waiting to be added **/,
                            &cloned_port_info                                   /** ptr to port info for cloned kernel **/
                        )
                    );
                }
                else
                {
                    /** 
                     * we found a match, lets link up the new 
                     * kernel.
                     */
                    d->temp_map->link(
                        cloned_kernel       /** which kernel **/,
                        port_info.my_name   /** port name    **/,
                        found->second        /** cloned destination **/,
                        port_info.other_name /** destination **/,
                        ( port_info.out_of_order ? raft::order::out : raft::order::in )
                    );

                }
                /** go through unmatched and see if we can match some up **/
            }
            /** go through unmatched and see if we can match some up **/
            updateUnmatched( d );
        }   /** end loop over current vertex output edges **/

        /** go through unmatched and see if we can match some up **/
        updateUnmatched( d );
    } );
    /** set up container to use BFT function **/
    std::set< raft::kernel* > source_kernels_container;
    source_kernels_container.insert( start );
    GraphTools::BFT( source_kernels_container, 
                     f,
                     GraphTools::input,
                     reinterpret_cast< void* >( &d ) );

    assert( d.unmatched.size() == 0 );
    assert( d.temp_map != nullptr );
    return( d.temp_map );
}

raft::temp_map*
GraphTools::duplicateFromVertexToSink( raft::kernel * const start )
{
    assert( start != nullptr );
    struct Data
    {
        Data() : temp_map( new raft::temp_map() )
        {
        }
        /**
         * NOTE: this map contains a reference from the pointers
         * cast to uintptr_t (set as the key) that are the original 
         * (to be duplicated kernels) to the new one which is the 
         * value. 
         */
        std::map< std::uintptr_t, raft::kernel * >      kernel_map;
        /** 
         * for graphs with a feedback loop, back-edges, we could
         * in fact have an edge that appears in the BFT whose
         * other terminal end has yet to be reached.
         */
        std::map< std::uintptr_t, PortInfo* >           unmatched;
        raft::temp_map                                 *temp_map     = nullptr;
    }   d;

    auto updateUnmatched( []( Data * const d ) -> void 
    {
            auto intersect( raft::utility::intersect_map(
                d->unmatched,
                d->kernel_map
            ) );
            for( auto &match: (*intersect ) )
            {
                /** 
                 * second will be unmatched portinfo then the kernel map
                 * we need to take the portinfo object that now has an 
                 * initialized source and set up its destination. The name
                 * thats given is already correct, just not pointing to the
                 * correct (cloned) kernel.
                 */
                auto *portinfo( match.second.first );
                /** double check to make sure that source kernel hasn't been deallocated **/
                assert( portinfo->my_kernel != nullptr );
                /** sanity check on key type, make sure somebody didn't accidentally update destination kernel **/
                assert( reinterpret_cast< std::uintptr_t >( portinfo->other_kernel ) == match.first );
                d->temp_map->link(
                    portinfo->my_kernel     /** cloned source **/,
                    portinfo->my_name       /** output port name inherited from clone parent **/,
                    match.second.second         /** newly found cloned destination kernel        **/,
                    portinfo->other_name,
                    ( portinfo->out_of_order ? raft::order::out : raft::order::in ) );
                //TODO, add overkill asserts for each of these
                d->kernel_map.erase( match.first );
                d->unmatched.erase( match.first );
            }
    } );

    /** make structure to hold previously cloned kernel **/
    vertex_func f( [&updateUnmatched]( raft::kernel *current,
                       void         *data ) -> void
    {
        Data *d( reinterpret_cast< Data* >( data ) );
        auto *cloned_kernel( current->clone() );
        if( cloned_kernel == nullptr )
        {
            //TODO throw an exception
            std::cerr << 
                "attempting to clone a kernel that wasn't meant to be cloned()\n";
            exit( EXIT_FAILURE );
        }
        /** first kernel **/
        if( d->kernel_map.size() == 0 )
        {
            const auto d_ret_val( d->kernel_map.insert( std::make_pair( 
                reinterpret_cast< std::uintptr_t >( current ),
                cloned_kernel ) ) );
#if NDEBUG
            UNUSED( d_ret_val );
#else
            assert( d_ret_val.second == true );
#endif
            return;
        }
        else
        {
            
            const auto d_ret_val( d->kernel_map.insert( std::make_pair( 
                reinterpret_cast< std::uintptr_t >( current ),
                cloned_kernel ) ) );
#if NDEBUG
            UNUSED( d_ret_val );
#else
            assert( d_ret_val.second == true );
#endif
            /** 
             * loop over output ports of "current" kernel and hook the
             * corresponding ports of the cloned kernel to the corresponding
             * ports on the cloned output kernels in the map (inside data 
             * struct).
             */
            auto &map_of_ports( current->input.portmap.map );
            for( auto &port : map_of_ports )
            {   
                /** port.first is the name **/
                auto &port_info( port.second );
                /**
                 * we want to link clone.out to the 
                 * output kernel clone saved in the 
                 * map that corresponds to the memory
                 * location then link it to the new 
                 * kernels port.
                 */
                const auto src_kern_hash( 
                    reinterpret_cast< std::uintptr_t >( 
                        port_info.other_kernel
                    )
                );
                /** 
                 * look up this source kernel in our
                 * clone map.
                 */
                auto found( d->kernel_map.find( src_kern_hash ) );
                if( found == d->kernel_map.end() )
                {
                    /** 
                     * get port info for the current named port, what we want is the cloned kernel's 
                     * port info then index it on the destination that we're waiting on, when the
                     * destination appears in the kernel_map then we can complete the link 
                     */
                    auto &cloned_port_info( cloned_kernel->input.getPortInfoFor( port_info.my_name ) );
                    /** destination not yet reached, likely back edge **/
                    d->unmatched.insert( 
                        std::make_pair( 
                            reinterpret_cast< std::uintptr_t >( src_kern_hash ) /** kern we're waiting to be added **/,
                            &cloned_port_info                                   /** ptr to port info for cloned kernel **/
                        )
                    );
                }
                else
                {
                    /** 
                     * we found a match, lets link up the new 
                     * kernel.
                     */
                    d->temp_map->link(
                        cloned_kernel       /** which kernel **/,
                        port_info.my_name   /** port name    **/,
                        found->second        /** cloned destination **/,
                        port_info.other_name /** destination **/,
                        ( port_info.out_of_order ? raft::order::out : raft::order::in )
                    );

                }
                /** go through unmatched and see if we can match some up **/
            }
            /** go through unmatched and see if we can match some up **/
            updateUnmatched( d );
        }   /** end loop over current vertex output edges **/

        /** go through unmatched and see if we can match some up **/
        updateUnmatched( d );
    } );
    /** set up container to use BFT function **/
    std::set< raft::kernel* > source_kernels_container;
    source_kernels_container.insert( start );
    GraphTools::BFT( source_kernels_container, 
                     f,
                     GraphTools::output,
                     reinterpret_cast< void* >( &d ) );

    assert( d.unmatched.size() == 0 );
    assert( d.temp_map != nullptr );
    return( d.temp_map );
}

void
GraphTools::__BFT( std::queue< raft::kernel* > &queue,
                   std::set<   raft::kernel* > &visited_set,
                   edge_func                   func,
                   const graph_direction       direction,
                   void                        *data,
                   bool                        connected_error )
{
   while( queue.size() > 0 )
   {
      auto *k( queue.front() );
      queue.pop();
      if( k == nullptr )
      {
        break;
      }
      auto &port_container( ( direction == GraphTools::output ? 
                        k->output :
                        k->input ) );
      /** iterate over all out-edges **/
      /** 1) get lock **/
      while( ! port_container.portmap.mutex_map.try_lock() )
      {
         std::this_thread::yield();
      }
      //we have lock, continue
      /** 2) get map **/
      auto &map_of_ports( port_container.portmap.map );
      for( auto &port : map_of_ports )
      {
         PortInfo &source( port.second );
         /** get dst edge to call function on **/
         if( source.other_kernel != nullptr  )
         {
            PortInfo &dst(
               source.other_kernel->input.getPortInfoFor( source.other_name ) );
            func( source, dst, data );
         }
         else
         if( connected_error )
         {
            std::stringstream ss;
            ss << "Unconnected port detected at " <<
               common::printClassName( *k ) <<
                  "[ \"" <<
                  source.my_name << " \"], please fix and recompile.";
            port_container.portmap.mutex_map.unlock();
            throw PortException( ss.str() );
         }
         /** if the dst kernel hasn't been visited, visit it **/
         if( visited_set.find( source.other_kernel ) == visited_set.end() )
         {
            queue.push( source.other_kernel );
            visited_set.insert( source.other_kernel );
         }
      }
      port_container.portmap.mutex_map.unlock();
   }
   return;
}

void
GraphTools::__BFT( std::queue< raft::kernel* > &queue,
                   std::set< raft::kernel*   > &visited_set,
                   vertex_func                 func,
                   const graph_direction       direction,
                   void                        *data )
{
   while( queue.size() > 0 )
   {
      auto *source( queue.front() );
      if( source == nullptr )
      {
         break;
      }
      queue.pop();
      /** iterate over all out-edges **/
      /** 1) get lock **/
      auto &port_container( ( direction == GraphTools::output ? 
                        source->output :
                        source->input ) );
        
      while( ! port_container.portmap.mutex_map.try_lock() )
      {
         std::this_thread::yield();
      }
      /** 2) get map **/
      auto &map_of_ports( port_container.portmap.map );
      /** 3) visit kernel **/
      func( source, data );
      /** 4) add children to queue **/
      for( auto &port : map_of_ports )
      {
         PortInfo &source( port.second );
         /** get dst edge to call function on **/
         if( source.other_kernel != nullptr  )
         {
            /** if the dst kernel hasn't been visited, visit it **/
            if( visited_set.find( source.other_kernel ) == visited_set.end() )
            {
               queue.push( source.other_kernel );
               visited_set.insert( source.other_kernel );
            }
         }
      }
      port_container.portmap.mutex_map.unlock();
   }
   return;
}

raft::temp_map*
GraphTools::duplicateBetweenVertices( raft::kernel * const start,
                                      raft::kernel * const end )
{
    assert( start != nullptr );
    assert( end   != nullptr );
    struct Data
    {
        Data() : temp_map( new raft::temp_map() )
        {
        }
        /**
         * NOTE: this map contains a reference from the pointers
         * cast to uintptr_t (set as the key) that are the original 
         * (to be duplicated kernels) to the new one which is the 
         * value. 
         */
        std::map< std::uintptr_t, raft::kernel * >      kernel_map;
        /** 
         * for graphs with a feedback loop, back-edges, we could
         * in fact have an edge that appears in the BFT whose
         * other terminal end has yet to be reached.
         */
        std::map< std::uintptr_t, PortInfo* >           unmatched;
        raft::temp_map                                 *temp_map     = nullptr;
    }   d;

    auto updateUnmatched( []( Data &d ) -> void 
    {
            auto intersect( raft::utility::intersect_map(
                d.unmatched,
                d.kernel_map
            ) );
            for( auto &match: (*intersect ) )
            {
                /** 
                 * second will be unmatched portinfo then the kernel map
                 * we need to take the portinfo object that now has an 
                 * initialized source and set up its destination. The name
                 * thats given is already correct, just not pointing to the
                 * correct (cloned) kernel.
                 */
                auto *portinfo( match.second.first );
                /** double check to make sure that source kernel hasn't been deallocated **/
                assert( portinfo->my_kernel != nullptr );
                /** sanity check on key type, make sure somebody didn't accidentally update destination kernel **/
                assert( reinterpret_cast< std::uintptr_t >( portinfo->other_kernel ) == match.first );
                d.temp_map->link(
                  portinfo->my_kernel     /** cloned source **/,
                  portinfo->my_name       /** output port name inherited from clone parent **/,
                  match.second.second         /** newly found cloned destination kernel        **/,
                  portinfo->other_name,
                  ( portinfo->out_of_order ? raft::order::out : raft::order::in ) );
                //TODO, add overkill asserts for each of these
                d.kernel_map.erase( match.first );
                d.unmatched.erase( match.first );
            }
    } );
    
    /** 
     * so this is going to be a special case of the BFS
     * where the entry has a single in-edge and the exit
     * has a single out-edge. The termination conditions
     * regardless of internal connectivity is when the 
     * ending vertex has been inserted into the queue a
     * number of times that is equal to the in-edge count
     * of that last vertex (i.e., all expected in-edges
     * have been found though all paths within the graph
     * which gets us out of the horrid end-case of all paths
     * if we had back edges from the last vertex...so 
     * restriction == faster, and our templated language
     * restricts us to that condition.
     */
    std::queue< raft::kernel * > queue;
    const auto                      term_cond( end->input.count() );
    decltype( end->input.count() )  terminate( 0 );
    queue.emplace( start );
    static_assert( 
        std::is_pointer< decltype( end ) >::value, 
            "runtime error: end must be a pointer" ); 
    const auto hash_end( reinterpret_cast< std::uintptr_t >(
        end
    ) );
    auto update_term_cond( 
        [ hash_end, &terminate ]( raft::kernel * const test ) noexcept -> 
            raft::kernel* 
    {
        const auto hash_curr( reinterpret_cast< std::uintptr_t >( test ) );
        terminate += ! ( hash_end ^ hash_curr );
        return( test );   
    } );
    std::set< std::uintptr_t > visited;
    while( queue.size() > 0 && terminate < term_cond )
    {
        auto *curr_ptr( queue.front() );
        queue.pop();
        assert( curr_ptr != nullptr );
        const auto k_hash( reinterpret_cast< std::uintptr_t >( 
            curr_ptr        
        ) );
        const auto ret_insert( visited.insert( k_hash ) );
        if( ! ret_insert.second  )
        {
            /** 
             * given back edges we could have added some things 
             * we need to match up. we already know the connectivity
             * so all we need is the other kernel to add to. Its
             * easy to construct an example on a dry erase board 
             * that results in new kernels being added from back 
             * edge and resulting in the one we need to connect
             * being available now 
             */
            updateUnmatched( d );
            continue;
        }
        auto *cloned_ptr( curr_ptr->clone() );
        d.kernel_map.insert( std::make_pair( k_hash, cloned_ptr ) );
        /**
         * reason we're using std::set vs. a visited bit on the 
         * kernel data structure is b/c we could have (and do)
         * multiple threads traversing this graph at any one
         * time each having the need for unique non-aliasable
         * "visited info." Having a single one would force
         * sequentializing the grapht raversals to a single
         * thread. Secondly a set, while being more computationally
         * taxing than a giant bit-vector array with a bit for
         * each vertex, this is far more compact since the 
         * sub-graph traversed in this case should be less
         * than the size of the total graph. Likely I'll change
         * the above representations to dynamic bit vectors
         * using either boost or custom since those in fact
         * (BFT and DFT) most often do in fact traverse the 
         * entire graph and the speed of a bitvector would 
         * outweight the space of having a bit per kernel.
         */
        if( curr_ptr != end )
        {
            auto &map_of_ports( curr_ptr->output.portmap.map );
            for( auto &port : map_of_ports )
            {   
                /** port.first is the name **/
                auto &port_info( port.second );
                /**
                 * we want to link clone.out to the 
                 * output kernel clone saved in the 
                 * map that corresponds to the memory
                 * location then link it to the new 
                 * kernels port.
                 */
                const auto dst_kern_hash( 
                    reinterpret_cast< std::uintptr_t >( 
                        port_info.other_kernel
                    )
                );
                /** 
                 * look up this source kernel in our
                 * clone map.
                 */
                auto found( d.kernel_map.find( dst_kern_hash ) );
                if( found == d.kernel_map.end() )
                {
                    /** 
                     * get port info for the current named port, 
                     * what we want is the cloned kernel's 
                     * port info then index it on the destination 
                     * that we're waiting on, when the
                     * destination appears in the kernel_map 
                     * then we can complete the link 
                     */
                    auto &cloned_port_info( 
                        cloned_ptr->input.getPortInfoFor( port_info.my_name ) );
                    /** destination not yet reached, likely back edge **/
                    d.unmatched.insert( 
                        std::make_pair( 
                            reinterpret_cast< std::uintptr_t >( dst_kern_hash ) /** kern we're waiting to be added **/,
                            &cloned_port_info                                   /** ptr to port info for cloned kernel **/
                        )
                    );
                }
                else
                {
                    /** 
                     * we found a match, lets link up the new 
                     * kernel.
                     */
                    d.temp_map->link(
                        cloned_ptr           /** which kernel       **/,
                        port_info.my_name    /** port name          **/,
                        found->second        /** cloned destination **/,
                        port_info.other_name /** destination        **/,
                        ( port_info.out_of_order ? raft::order::out : raft::order::in )
                    );

                }
                /** go through unmatched and see if we can match some up **/
            }
        }  /** end if curr_ptr != end **/ 
        /** go through unmatched and see if we can match some up **/
        updateUnmatched( d );
    } /** keep going  while not term condition**/
    updateUnmatched( d );

    return( d.temp_map );
}

void
GraphTools::__DFT( std::stack< raft::kernel* > &stack,
                   std::set<   raft::kernel* > &visited_set,
                   edge_func                   func,
                   void                        *data )
{
   UNUSED( stack );
   UNUSED( visited_set );
   UNUSED( func );
   UNUSED( data );

   /** TODO, implement me **/
   assert( false );
}

void
GraphTools::__DFT( std::stack< raft::kernel* > &stack,
                   std::set<   raft::kernel* > &visited_set,
                   vertex_func                 func,
                   void                        *data )
{
   UNUSED( stack );
   UNUSED( visited_set );
   UNUSED( func );
   UNUSED( data );

   /** TODO, implement me **/
   assert( false );
}
