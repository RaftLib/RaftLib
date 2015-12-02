/**
 * search.tcc - thin wrapper around boost's search
 * algorithms for the moment.  I might add my own
 * rabin-karp as well, however boyer-moore and b-m-h
 * are much more efficient in general usage.
 *
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
#ifndef _SEARCH_TCC_
#define _SEARCH_TCC_  1

#include <raft>
#include <boost/algorithm/searching/boyer_moore.hpp>

namespace raft
{

enum searchalgo { rk, bm, bmh, kpm };

template < class T, searchalgo ALGO > class search;

template < class T > class search< searchalgo::bm >
{
   namespace bt = boost::algorithm;
public:
   search( const T &&pattern ) : se( pattern.begin(), pattern.end() )
   {
       
   }

   virtual raft::kstatus run()
   {
      auto &input_port( (this)->input[ "in" ] );
      auto &corpus( input_port.template peek< T >() );
      for( auto it( se( corpus.begin(), corpus.end() ) );
         it != corpus.end(); it = se( it, corpus.end() ) )
      {
         
      }
      input_port.unpeek();
      input_port.recycle( 1 );
      return( raft::proceed );
   }
private:
   using type_t = 
      typename std::remove_reference< decltype( pattern.begin() ) >::type;
   bt::boyer_moore< type_t > se;
};

}

#endif /* END _SEARCH_TCC_ */
