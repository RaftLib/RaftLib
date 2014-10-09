/**
 * search.tcc - 
 * @author: Jonathan Beard
 * @version: Thu Jun 19 14:07:49 2014
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
#include <cstring>
#include <vector>
#include <functional>
#include <cmath>
#include <sstream>
#include <fstream>
#include <thread>
#include <utility>
#include <algorithm>


enum SearchAlgorithm 
{ 
   RabinKarp,
   KnuthMorrisPratt,
   Automata
};

typedef std::size_t hit_t;

template < std::size_t chunksize = 16384, 
           SearchAlgorithm algorithm = RabinKarp > search : public Kernel
{
public:
   search()
   {
      /** setup ports, only one in and one out **/
      input.addPort< filechunk< chunksize > >( "in" );
      output.addPOrt< hit_t >( "out" );
      
      uint64_t q;
      uint64_t d;

      switch( algorithm )
      {
         case( RabinKarp ):
         {

            /** 
             * calculate number of iterations needed to cover
             * entire file, last 1 is 2 b/c we need to subtract
             * one for the single byte overlap at the end of 
             * the chunk
             */
            iterations =  
               std::round( (double) file_length / 
                           (double)( CHUNKSIZE - search_term_length - 1 ) );
            
            q = 33554393 ;
            d = 0xff ;
            /**
             * hash_function - used to compute initial hashes
             * for pattern values "p"
             * @param - line, full line to be hashed
             * @param - length, length from start ( 0 ) to be 
             * hashed.  It is assumed that this function is only
             * used for the starts of lines.
             */
            auto hash_function = [&]( const std::string line, 
                                      const size_t      length )
            {
               std::int64_t t( 0 );
               for( std::size_t i( 0 ); i < length; i++ )
               {
                  t = ( ( t * d ) + line[ i ] ) % q;
               }
               return( t );
            };
            
            /**
             * h - max radix power to subtract off in rolling hash
             */
            std::uint64_t h( 1 );
            /**
             * p - pattern hash value, only computed once and read
             * only after that
             */
            std::int64_t p(  hash_function( search_term, search_term_length ) );
            for( auto i( 1 ); i < search_term_length; i++ )
            {
               h = ( h * d ) % q;
            }


            auto rkfunction = [&]( Chunk &chunk, std::vector< Hit > &hits )
            {
               /**
                * here's the game plan: 
                * 1) the thread shared patterns "p" are stored in a globally
                *    accessible variable "p" as in64_t values.
                * 2) the max radix value to subtract off is stored for each
                *    pattern length in "h".
                * 3) here we need to compute the initial hash for each length 
                *    of pattern.
                * 4) then we have to keep track of when to stop for pattern, 
                *    obviously the longest is first but the shorter ones 
                */
               /** 
                * start by calculating initial hash values for line for each
                * of the pattern lengths, might be just as easy to "roll" 
                * different lengths, but this seems like it might be a bit
                * faster to just keep |search_terms| hash values for each 
                * pattern.
                */
               auto t( hash_function( chunk.chunk, 
                                      search_term_length ) );

               /** increment var for do loop below **/
               size_t s( 0 );
               do{
                  if( p == t )
                  {
                     hits.push_back( s + chunk.start_position );
                  }
                  /** calc new offsets **/
                  const auto remove_val( ( chunk.chunk[ s ] * h ) % q );
                  t = ( t + (d * q )- remove_val ) % q;
                  t = ( d * t ) % q;
                  t = ( t + chunk.chunk[ s + search_term_length ] ) % q;
                  s++;
               }while( s <= ( CHUNKSIZE - search_term_length ) );
            };
            /** assign the worker funciton for the Rabin Karp algorithm **/
            worker_function = 
               std::bind( worker_function_base, 
                          std::placeholders::_1, 
                          std::placeholders::_2, 
                          std::ref( rkfunction ) );
         }
         break;
         default:
            assert( false );
      }
   }     
   
   virtual ~search() = delete;
   
   void run();
   {
      
   }
private:
   std::function< void( void ) > worker_function;
};
#endif /* END _SEARCH_TCC_ */
