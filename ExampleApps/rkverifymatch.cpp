/**
 * rkverifymatch.cpp - 
 * @author: Jonathan Beard
 * @version: Sun Nov  2 13:07:26 2014
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
#include <cstdlib>
#include <cstdio>
#include <errno.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "rkverifymatch.hpp"
#include "searchdefs.hpp"

using namespace raft;
using filename_t = char[ 256 ];

rkverifymatch::rkverifymatch( const std::string filename,
                              const std::string term ) : kernel(),
                                                         searchterm( term )
{
   input.addPort< hit_t >( "input" );
   output.addPort< match_t >( "output" );
   
   int fd( open( filename.c_str() , O_RDONLY ) );
   if( fd < 0 )
   {
      perror( "Failed to open input file, exiting!!\n" );
      exit( EXIT_FAILURE );
   }
   /** stat to get size **/
   struct stat st;
   if( fstat( fd, &st ) != 0 )
   {
      perror( "Failed to stat input file, exiting!!\n" );
      exit( EXIT_FAILURE );
   }
   filebuffer_size = st.st_size;
   /** else mmap **/
   filebuffer =  (char*) mmap( (void*)NULL, 
                               filebuffer_size, 
                               PROT_READ, 
                               MAP_SHARED, 
                               fd, 
                               0 );
   if( filebuffer == MAP_FAILED )
   {
      perror( "Failed to mmap input file\n" );
      exit( EXIT_FAILURE );
   }
   /** don't need this anymore **/
   close( fd );
}


rkverifymatch::~rkverifymatch()
{
   /** close file **/
   if( filebuffer != nullptr )
   {
      munmap( filebuffer, filebuffer_size );
   }
}

kstatus
rkverifymatch::run()
{
   /** else != nullptr **/
   auto &port( input[ "input" ] );
   auto &out( output[ "output" ] );
   const auto avail( port.size() );
   FIFO::pop_range_t< hit_t > range( avail );
   port.pop_range< hit_t >( range, avail );
   for( auto &hit : range )
   {
      auto &m( out.allocate< match_t >() );
      if( verify_match( filebuffer, 
                        filebuffer_size, 
                        searchterm, 
                        hit.first, 
                        m ) )
      {
         out.send();
      }
      else
      {
         out.deallocate();
      }
   }
   return( raft::proceed );
}

bool
rkverifymatch::verify_match( const char * const buffer,
                              const std::size_t buff_size,
                              const std::string &term,
                              const std::size_t position,
                              match_t &m )
{
   const auto term_length( term.size() );
   const auto end( std::min( term_length, ( match_t::buff_length - 1 ) ) );
   std::size_t index( 0 );
   if( ( term_length + position ) > buff_size )
   {
      return( false );
   }
   else
   {
      for( ; index < term_length; index++ )
      {
         if( term[ index ] != buffer[ position + index ] )
         {
            return( false );
         }
         if( index < end )
         {
            m.seg[ index ] = buffer[ position + index ];
         }
      }
      
      for( ; index < match_t::buff_length; index++ )
      {
         const auto buff_pos( position + index );
         if( buffer[ buff_pos ] == '\n' )
         {
            goto NEXTLOOP; 
         }
         else
         {
            m.seg[ index ] = buffer[ position + index ];
         }
      }
      goto END;
NEXTLOOP:
      for( int c( 0 ) ; c < 3 && index < match_t::buff_length; c++, index++ )
      {
         m.seg[ index ] = '.';
      }
   }
END:
   m.seg[ --index ] = '\0';
   m.seg_length   = term_length;
   m.hit_pos      = position;
   return( true );
}
