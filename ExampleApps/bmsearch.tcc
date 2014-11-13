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
      bad_char( searchterm.c_str(), m, bad_char_arr );
   }

   virtual ~search()
   {
   }

   virtual raft::kstatus run() 
   {
      auto &in_port( input[ "in" ] );
      const auto n( in_port.size() );
      auto everything( in_port.peek_range< char >( n ) );
      /** in this case, we know the buffer is contiguous **/
      char * const buff_ptr( (char*)&( everything[ 0 ] ) );
      std::int64_t s( 0 );
      auto &out_port( output[ "out" ] );
      const auto index( everything.getindex() );
      while( s <= ( n - m ) )
      {
         std::int64_t j( m - 1 );
         while( j >= 0 && pat[ j ] == buff_ptr[ s + j ] )
         {
            j--;
         }
         if( j < 0 )
         {
            out_port.push< hit_t >( s + everything.getindex() );
            s += (  ( ( s + m ) < n ) ? m - bad_char_arr[ buff_ptr[ s + m ] ] : 1 );
         }
         else
         {
            s += std::max( (std::int64_t )1, 
                           (std::int64_t ) j - bad_char_arr[ buff_ptr[ s + j ] ] );
         }

      }
      return( raft::stop );
   }
   
private:
   const static std::size_t    NO_OF_CHARS = 256;
   std::int64_t  bad_char_arr[ NO_OF_CHARS ];
   const std::int64_t          m           = 0;
   const std::string           pat;
   
   
   static void bad_char( const char * const str,
                         const std::size_t  size,
                         std::int64_t       badchar[ NO_OF_CHARS ] )
   {
      for( auto i( 0 ); i < NO_OF_CHARS; i++ )
      {
         badchar[ i ] = -1;
      }
      for( auto i( 0 ); i < size; i++ )
      {
         badchar[ (int) str[ i ] ] = i;
      }
   }
};
#endif /* END _BMSEARCH_TCC_ */
