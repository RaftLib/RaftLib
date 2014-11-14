/**
 * bmsearch.tcc - 
 * @author: Jonathan Beard
 * @version: Sun Nov  9 17:09:08 2014
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
#ifndef _BMSEARCH_TCC_
#define _BMSEARCH_TCC_  1
#include <raft>
#include <functional>
#include <cstddef>
#include <cstdint>
#include <cmath>

#include "searchdefs.hpp"
#include <cassert>


template <> class search< boyermoore > : public kernel
{
public:
   search( const std::string searchterm ) : kernel(),
                                            pat( searchterm ),
                                            m( searchterm.length() )
   {
      /**
       * there's an infinite loop with very large files,
       * probably an overflow in the run with the s,n or m
       * fix in the morning 
       */
      assert( searchterm.length() > 0 ); 
      input.addPort< char  >( "in"  );
      output.addPort< hit_t   >( "out" );
      bad_char( searchterm, 256, &shift_table );
      assert( shift_table != nullptr );
   }

   virtual ~search()
   {
      delete[]( shift_table );
   }

   virtual raft::kstatus run() 
   {
      auto &in_port( input[ "in" ] );
      const std::int64_t n( in_port.size() );
      if( m > n )
      {
         return( raft::stop );
      }
      auto everything( in_port.peek_range< char >( n ) );
      /** in this case, we know the buffer is contiguous **/
      char * const buff_ptr( (char*)&( everything[ 0 ] ) );
      std::int64_t s( 0 );
      /** ref to output port so we don't have to look it up cont.**/
      auto &out_port( output[ "out" ] );
      /** start index of text **/
      const auto index( everything.getindex() );
      /** rightmost index of pattern **/
      std::int64_t r_index( m - 1 );
      while( r_index <= n - 1 )
      {
         std::int64_t k( 0 );
         while( k <= m-1 && pat[ m - 1 - k ] == buff_ptr[ r_index - k ] )
         {
            k++;
         }
         if( k == m )
         {
            out_port.push< hit_t >( r_index - m + 1 + index );
            r_index++; 
         }
         else
         {
            r_index = r_index + shift_table[ (std::size_t) buff_ptr[ r_index ] ];
         }
      }
      return( raft::stop );
   }
   
private:
   std::int64_t               *shift_table = nullptr ;
   const std::string           pat;
   const std::int64_t          m           = -1;
   
   
   static void bad_char( const std::string  &pattern,
                         const std::size_t   alphabet_size,
                         std::int64_t      **shift_table )
   {
      assert( pattern.size() > 0 );
      const auto m( pattern.size() );
      *shift_table = new std::int64_t [ alphabet_size ];
      for( std::int64_t i( 0 ); i < alphabet_size; i++ )
      {
         (*shift_table)[ i ] = m;
      }
      for( std::int64_t j( 0 ); j < m - 1; j++ )
      {
         (*shift_table)[ (std::size_t) pattern[ j ] ] = ( m - 1 - j );
      }
   }
};
#endif /* END _BMSEARCH_TCC_ */
