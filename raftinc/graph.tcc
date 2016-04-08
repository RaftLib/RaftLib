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
#include <ostream>

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

   static 
   std::ostream& print( std::ostream &stream, const ScotchTables< T > &table )
   {
      stream << "Vertices: \n";
      for( auto i( 0 ); i < table.num_vertices; i++ )
      {
         stream << table.vtable[ i ] << "  ";
      }
      stream << "\n\n";
      stream << "Edges: \n";
      for( auto i( 0 ); i < table.num_edges; i++ )
      {
         stream << table.etable[ i ] << "  ";
      }
      stream << "\n\n";
      stream << "Weights: \n";
      for( auto i( 0 ); i < table.num_edges; i++ )
      {
         stream << table.eweight[ i ] << "  ";
      }
      stream << "\n";
      return( stream );
   }
};

template < typename EDGETYPE, typename WEIGHT > class graph;

using weight_t = std::int32_t;

template <> class graph< weight_t, weight_t >
{

private:
   /**
    * wt - struct to hold the destination of each edge,
    * and the weight associated with the src-dst combination
    */
   struct wt
   {
      /**
       * wt - constructor
       * @param   dst - const weight_t, destination of appropriate source vertex
       * @param   weight - const weight_t, weight associated with this edge (arc)
       */
      constexpr wt( const weight_t dst,
                    const weight_t weight ) : dst( dst ),
                                              weight( weight ){}

      /** copy constructor **/
      constexpr wt( const wt &other ) : dst( other.dst ),
                                        weight( other.weight ){}

      /** needed for find operation **/
      constexpr bool operator == ( const wt &other )
      {
         return( dst == other.dst );
      }

      const weight_t dst;
      const weight_t weight;
   };
  
   /**
    * __addEdge - helper method for same named
    * function above, see its documentation.
    */
   void __addEdge( const weight_t src,
                   const weight_t dst,
                   const weight_t weight )
   {
      auto it( edgelist.find( src ) );
      wt w( dst, weight );  
      if( it == edgelist.end() )
      {
         auto *v( new std::vector< wt >() );
         v->emplace_back( w );
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
         (*it).second->emplace_back( w );
         /** we know the source is already there, just need to check the dst **/
         if( vertex_hash.find( dst ) == vertex_hash.end() )
         {
            vertex_hash.insert( dst );
         }
      }
      return;
   }
   /** edge adjacency list **/
   std::map< weight_t, std::vector< wt >* > edgelist; 
   /** 
    * simplifies counting the number of vertices.
    * TODO, recode with counter...if this gets large
    * it could get really really large
    */
   std::set< weight_t >                     vertex_hash;
public:


   /** don't need anything special **/
   graph()  = default;

   /** destructor **/
   virtual ~graph()
   {
      for( auto &pair : edgelist )
      {
         delete( pair.second );
      }
   }

   /**
    * addEdge - add an edge for each "edge" in the actual 
    * graph, pretty self explanatory.  Weights are whatever
    * you want them to be.  The function will add a back
    * edge to the underlying graph to be compatible with
    * the Scotch partitioning framework.
    * @param   src - const weight_t, source
    * @param   dst - const std;:int32_t, destination
    * @param   weight - const weight_t weight
    */
   void addEdge( const weight_t src,
                 const weight_t dst,
                 const weight_t weight )
   {
      /** forward edge **/
      __addEdge( src, dst, weight );
      /** add back edge **/
      __addEdge( dst, src, weight );
      return;
   }


   /**
    * getScotchTables() - call once you are completely done
    * adding edges to the graph, formats the returned arrays
    * accodingly.  The returned object will release the memory
    * allocated once it leaves the current frame.
    * @return ScotchTables< weight_t >
    */
   ScotchTables< weight_t >
   getScotchTables()
   {
      const auto size( vertex_hash.size() );
      weight_t *vertex_list = new weight_t[ size + 1  ];
      std::vector< weight_t > edge_list_temp;
      std::vector< weight_t > edge_list_weight_temp;
      auto vertex_list_index( 0 );
      auto edge_index( 0 );
      const auto first_vertex_index( *vertex_hash.cbegin() );
      for( const auto vertex_id : vertex_hash )
      {
         const auto out_edge_size( edgelist[ vertex_id ]->size() );
         vertex_list[ vertex_list_index++ ] = edge_index;
         std::vector< wt > &dstlist( (*edgelist[ vertex_id ]) );
         for( const auto &edge : dstlist )
         {
            edge_list_temp.emplace_back( edge.dst - first_vertex_index );
            edge_list_weight_temp.emplace_back( edge.weight );
         }
         edge_index += out_edge_size;
      }
      /** one past **/
      vertex_list[ size ] = edge_index;
      const auto edge_list_temp_size( edge_list_temp.size() );
      weight_t *edge_list = new weight_t[ edge_list_temp_size ];
      weight_t *edge_weight = new weight_t[ edge_list_temp_size ];
      for( auto i( 0 ); i < edge_list_temp_size; i++ )
      {
         edge_list[ i ]    = edge_list_temp[ i ];
         edge_weight[ i ]  = edge_list_weight_temp[ i ];
      }
      ScotchTables< weight_t >  table;
      table.vtable            = vertex_list;
      table.etable            = edge_list;
      table.eweight           = edge_weight;
      table.num_vertices      = size;
      table.num_edges         = edge_list_temp_size;
      table.partition         = new weight_t[ size ];
      return( table );
   }
   
   std::ostream& print( std::ostream &stream )
   {  
      for( const auto vertex : vertex_hash )
      {
         for( const auto &edge : *edgelist[ vertex ] )
         {
            stream << "Edge: " << vertex << " -> " << edge.dst << "\n";
         }
      }
      return( stream );
   }

   const decltype( vertex_hash )
   getVertexNumbersAtIndicies() 
   {
      return( vertex_hash ); 
   }

};

} /** end namespace raft **/
#endif /* END _GRAPH_TCC_ */
