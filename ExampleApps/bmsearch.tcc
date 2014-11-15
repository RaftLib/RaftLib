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
      assert( searchterm.length() > 0 ); 
      input.addPort< char  >( "in"  );
      output.addPort< hit_t   >( "out" );
   }

   virtual ~search()
   {
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
      unsigned char *buff_ptr( (unsigned char *)&( everything[ 0 ] ) );
      /** ref to output port so we don't have to look it up cont.**/
      auto &out_port( output[ "out" ] );
      /** start index of text **/
      const auto index( everything.getindex() );
      boyermoore_horspool( buff_ptr, 
                           n, 
                           (unsigned char*) pat.c_str(),
                           m,
                           index,
                           out_port );
      return( raft::stop );
   }
   
private:
   const std::string           pat;
   const std::size_t          m = 0;

   static
   void boyermoore_horspool( const unsigned char * N,   
                             std::size_t n,  
                             const unsigned char * M,    
                             const std::size_t m,
                             const std::size_t index,
                             FIFO   &port )  
   {  
       const unsigned char * const lb( N + n - 1 );
       std::size_t scan( 0 );  
       size_t bad_char_skip[ UCHAR_MAX + 1 ] __attribute__ ((aligned (16))) ;  
     
       if( m <= 0 || ! N || ! M || m > n )  
       {
         return; 
       }
       for(scan = 0; scan <= UCHAR_MAX; scan++ )
       {
         bad_char_skip[scan] = m;  
       }
       const std::size_t last( m - 1 );  
       for (scan = 0; scan < last; scan++ ) 
       {
         bad_char_skip[ M[ scan ] ] = last - scan; 
       }
       while( n >= m )  
       {  
           for(scan = last; N[scan] == M[scan]; scan = scan - 1)  
           {  
               if (scan == 0)   
               {  
                  port.push< hit_t >( lb - N + index);
                  break;
               }
           }  
           const auto skip_val( bad_char_skip[ N[ scan ] ] );
           n -= skip_val;  
           N += skip_val;
       }  
   }  
};
#endif /* END _BMSEARCH_TCC_ */
