/**
 * graph.tcc - 
 * @author: Jonathan Beard
 * @version: Tue Mar 10 12:43:47 2015
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
#ifndef _GRAPH_TCC_
#define _GRAPH_TCC_  1
#include <cstddef>
#include <cstdint>
#include <map>
#include <vector>
#include <set>
#include <utility>

namespace raft
{
template< typename T > struct ScotchTables
{
   ScotchTables() = default;
   ~ScotchTables()
   {
      delete[]( vtable );
      delete[]( etable );
      delete[]( eweight );
      delete[]( partition );
   }
   
   T              *vtable     = nullptr;
   T              *etable     = nullptr;
   T              *eweight    = nullptr;
   T              *partition  = nullptr;
   std::size_t    num_vertices;
   std::size_t    num_edges;
};

template < typename EDGETYPE, typename WEIGHT > class graph;

template <> class graph< std::int32_t, std::int32_t >
{
public:

   graph()  = default;

   ~graph()
   {
      for( auto &pair : edgelist )
      {
         delete( pair.second );
      }
   }

   void addEdge( const std::int32_t src,
                 const std::int32_t dst,
                 const std::int32_t weight )
   {
      /** forward edge **/
      __addEdge( src, dst, weight );
      /** add back edge **/
      __addEdge( dst, src, weight );
      return;
   }


   ScotchTables< std::int32_t >
   getScotchTables()
   {
      const auto size( vertex_hash.size() );
      std::int32_t *vertex_list = new std::int32_t[ size + 1  ];
      std::vector< std::int32_t > edge_list_temp;
      std::vector< std::int32_t > edge_list_weight_temp;
      auto vertex_list_index( 0 );
      auto edge_index( 0 );
      for( const auto vertex_id : vertex_hash )
      {
         std::cerr << vertex_id << "\n";
         const auto out_edge_size( edgelist[ vertex_id ]->size() );
         vertex_list[ vertex_list_index++ ] = edge_index;
         std::vector< wt > &dstlist( (*edgelist[ vertex_id ]) );
         for( const auto &edge : dstlist )
         {
            edge_list_temp.push_back( edge.dst );
            edge_list_weight_temp.push_back( edge.weight );
         }
         edge_index += out_edge_size;
      }
      /** one past **/
      vertex_list[ size ] = edge_index;
      const auto edge_list_temp_size( edge_list_temp.size() );
      std::int32_t *edge_list = new std::int32_t[ edge_list_temp_size ];
      std::int32_t *edge_weight = new std::int32_t[ edge_list_temp_size ];
      for( auto i( 0 ); i < edge_list_temp_size; i++ )
      {
         edge_list[ i ]    = edge_list_temp[ i ];
         edge_weight[ i ]  = edge_list_weight_temp[ i ];
      }
      ScotchTables< std::int32_t >  table;
      table.vtable            = vertex_list;
      table.etable            = edge_list;
      table.eweight           = edge_weight;
      table.num_vertices      = size;
      table.num_edges         = edge_list_temp_size;
      table.partition         = new std::int32_t[ size ];
      return( table );
   }


private:
   struct wt
   {
      wt( const std::int32_t dst,
          const std::int32_t weight ) : dst( dst ),
                                        weight( weight )
      {
         /** nothing to do here **/
      }

      wt( const wt &other ) : dst( other.dst ),
                              weight( other.weight )
      {
      }

      bool operator == ( const wt &other )
      {
         return( dst == other.dst );
      }

      std::int32_t dst    = 0;
      std::int32_t weight = 0;
   };
   
   void __addEdge( const std::int32_t src,
                   const std::int32_t dst,
                   const std::int32_t weight )
   {
      auto it( edgelist.find( src ) );
      wt w( dst, weight );  
      if( it == edgelist.end() )
      {
         auto *v( new std::vector< wt >() );
         v->push_back( w );
         edgelist.insert(
            std::make_pair( src, 
                            v ) );
         vertex_hash.insert( src );
         /** we know the src hasn't been seen, don't know about the dst **/
         if( vertex_hash.find( dst ) == vertex_hash.end() )
         {
            vertex_hash.insert( dst );
         }
      }
      else /** src already in graph **/
      {
#if DEBUG
         const auto &local_vector( *(*it).second );
         assert( std::find( vector.begin(), vector.end(), dst ) == local_vector.end() );
#endif
         (*it).second->push_back( w );
         /** we know the source is already there, just need to check the dst **/
         if( vertex_hash.find( dst ) == vertex_hash.end() )
         {
            vertex_hash.insert( dst );
         }
      }
      return;
   }

   std::map< std::int32_t, std::vector< wt >* > edgelist; 
   std::set< std::int32_t >                     vertex_hash;
};

} /** end namespace raft **/

#endif /* END _GRAPH_TCC_ */
