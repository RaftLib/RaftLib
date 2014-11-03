/**
 * search.tcc - 
 * @author: Jonathan Beard
 * @version: Sun Nov  2 06:23:18 2014
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
#ifndef _SEARCH_TCC_
#define _SEARCH_TCC_  1
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cmath>

#include <raft>
#include <raftio>

namespace raft
{
   using hit_t    = std::size_t;
   using chunk_t  = filechunk< 4096 >;
   enum searchalgorithm { rabinkarp, 
                          kmp,
                          automata };

template < searchalgorithm alg > class search;

template <> class search< rabinkarp > : public kernel
{
public:
   search( const std::string term ) : kernel(),
                                      search_term_length( term.length() ),
                                      pattern( hashfunc( term, search_term_length, d, q ) ),
                                      h( comp_h( q, d, search_term_length, h ) )
   {
      assert( search_term_length > 0 ); 
      input.addPort<  chunk_t >( "in"  );
      output.addPort< hit_t   >( "out" );
   }
   
   virtual kstatus run()
   {
      auto text( input[ "in" ].pop_s< chunk_t >() );
      const auto term_cond( std::min( chunk_t::getChunkSize(), (*text).length ) );
      std::size_t s( 0 );
      const char * const buffer = (*text).buffer;

      std::int64_t t( hashfunc( buffer, search_term_length, d, q ) ); 
      std::vector< hit_t > hits;
      do{
         if( pattern == t )
         {
            hits.push_back( s + (*text).start_position );
         }
         const auto remove_val( ( buffer[ s ] * h ) % q );
         t = ( t + ( d * q ) - remove_val ) % q;
         t = ( d * t ) % q;
         t = ( t + buffer[ s + search_term_length ] ) % q;
         s++;
      }while( s <= ( term_cond - search_term_length) );
      if( hits.size() > 0 )
      {
         output[ "out" ].insert( hits.begin(), hits.end() );
      }
      return( raft::proceed );
   }

private:
   /** prime number **/
   const std::uint64_t q = 33554393;
   /** shift **/
   const std::uint64_t d = 0xff;
   /** search term length **/
   const std::size_t   search_term_length = 0;
   /** term hash pattern **/
   const std::int64_t  pattern;
   /** max radix power to subtract from rolling hash **/
   const std::int64_t  h = 1;

   /** functions **/
   /**
    * hashfunc - rabinkarp rolling hash function, used once for the term
    * and to start off each chunk that the kernel intakes.
    * @param   text - const std::string, text to be hashed
    * @param   length - const std::size_t, length of string assumed to start at zero
    * @return  std::int64_t
    */
   static auto hashfunc( const std::string &text, 
                         const std::size_t length, 
                         const std::uint64_t d,
                         const std::uint64_t q ) -> std::int64_t
   {
      std::int64_t t( 0 );
      for( std::int64_t i( 0 ); i < length; i++ )
      {
         t = ( ( t * d ) + text[ i ] ) % q;
      }
      return( t );
   }

   template< typename Q, 
             typename D, 
             typename H, 
             typename L > static H comp_h( const Q q, 
                                              const D d, 
                                              const L l, 
                                              const H h )
   {
      H h2( 1 );
      for( auto i( 1 ); i < l; i++ )
      {
         h2 = ( h2 * d ) % q;
      }
      return( h2 );
   }
};


} /** end namespace raft **/
#endif /* END _SEARCH_TCC_ */
