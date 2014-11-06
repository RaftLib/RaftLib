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
#include <cstring>

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
#if 1    
   auto term_length( term.size() );
   if( ( term_length + position ) > buff_size )
   {
      return( false );
   }
   char *ptr(      (char*) &buffer[ position ] );
   char *term_ptr( (char*) term.c_str() );
   for( ;; )
   {
      if( term_length >= 8 )
      {
         std::uint64_t a_v = *((std::uint64_t*)ptr);
         std::uint64_t b_v = *((std::uint64_t*)term_ptr);
         if( a_v != b_v ){ return( false ); }
         ptr         += sizeof( std::uint64_t );
         term_ptr    += sizeof( std::uint64_t );
         term_length -= sizeof( std::uint64_t );
      }
      else if( term_length >= 4 )
      {
         std::uint32_t a_v = *((std::uint32_t*)ptr);
         std::uint32_t b_v = *((std::uint32_t*)term_ptr );
         if( a_v != b_v ){ return( false ); }
         ptr         += sizeof( std::uint32_t );
         term_ptr    += sizeof( std::uint32_t );
         term_length -= sizeof( std::uint32_t );
      }
      else if( term_length > 0 )
      {
         for( ; *term_ptr != '\0'; ++ptr, ++term_ptr, term_length-- )
         {
            if( *ptr != *term_ptr )
            {
               return( false );
            }
         }
      }
      else
      {
         break;
      }
   }
#else
   if( std::strncmp( &buffer[ position ], term.c_str(), term.length() ) != 0 )
   {
      return( false );
   }

#endif
   m     = position;
   return( true );
}
