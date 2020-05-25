/**
 * @author: Jonathan Beard
 * @version: Sat Aug 15 21:00:44 2015
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
#ifndef RAFTSEARCH_TCC
#define RAFTSEARCH_TCC  1

#include <utility>
#include <cstddef>
#include <raft>
#include <algorithm>

namespace raft
{
    
using match_t = std::pair< std::size_t /** start **/, 
                           std::size_t /** end   **/>; 

enum searchalgo { stdlib, pcre };

template < class T, searchalgo ALGO > class search;

template < class T > class search< T, stdlib > : public raft::kernel
{
public:
   search( const std::string && term ) : raft::kernel(),
                                         term_length( term.length() ),
                                         term( term )
   {
      input.addPort< T >( "0" );
      output.addPort< match_t >( "0" );
   }
   
   search( const std::string &term ) : raft::kernel(),
                                       term_length( term.length() ),
                                       term( term )
   {
      input.addPort<  T >( "0" );
      output.addPort< match_t >( "0" );
   }

   virtual ~search() = default;

   virtual raft::kstatus run()
   {
      auto &chunk( input[ "0" ].template peek< T >() );
      auto it( chunk.begin() );
      do
      {
         it = std::search( it, chunk.end(),
                           term.begin(), term.end() );
         if( it != chunk.end() )
         {
            const std::size_t loc( it.location() + chunk.start_position );
            const std::size_t end( loc + term_length );
            output[ "0" ].push( std::make_pair( loc, end ) );
            it += 1;
         }
         else
         {
            break;
         }
      }
      while( true );
      input[ "0" ].unpeek();
      input[ "0" ].recycle( );
      return( raft::proceed );
   }
private:
   const std::size_t term_length;
   const std::string term;
};



}

#endif /* END RAFTSEARCH_TCC */
