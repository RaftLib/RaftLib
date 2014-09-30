/**
 * fileio.tcc - 
 * @author: Jonathan Beard
 * @version: Mon Sep 29 14:24:00 2014
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
#ifndef _FILEIO_TCC_
#define _FILEIO_TCC_  1
#include <cstdlib>
#include <cstdio>
#include <cstddef>
#include <cstring>
#include <sys/stat.h>
#include <iostream>

#include <raft>

template < std::size_t size > struct filechunk
{
   filechunk() = default;

   filechunk( const filechunk< size > &other )
   {
      std::memcpy( buffer, other.buffer, other.length + 1 );
      start_position = other.start_position;
      length = other.length;
      CHUNKSIZE = other.size;
   }

   char              buffer[ size ]; 
   std::size_t       BUFFERSIZE;
   std::size_t       start_position;
   std::size_t       length;
   std::size_t       CHUNKSIZE = size;

   friend std::ostream& operator <<( std::ostream &output, filechunk< size > &c )
   {
      output << c.buffer;
      return( output );
   }
};

template < class chunktype = filechunk< 65536 >, 
           bool copy = false > class filereader : public Kernel
{
public:
   filereader( const std::string inputfile, 
               const std::size_t n_output_ports = 1,
               const std::size_t chunk_offset = 0 )
   {
      const std::string prefix( "output_" );
      for( auto index( 0 ); index < n_output_ports; index++ )
      {
         /** add a port for each index var, all named "output_#" **/
         output.addPort< chunktype  >( prefix + std::to_string( index ) );
      }
      
      /** stat file **/
      struct stat st;
      std::memset( &st, 0, sizeof( struct stat ) );
      if( stat( inputfile.c_str(), &st ) != 0 )
      {
         perror( "Failed to stat input file, exiting!" );
         //TODO, figure out global shutdown procedure
         exit( EXIT_FAILURE );
      }
      
      /** initialize file **/
      fp = fopen( inputfile.c_str() , "r" );
      if( fp == nullptr )
      {
         perror( "Failed to open input file, exiting!" );
         exit( EXIT_FAILURE );
      }
      
      /** get length in bytes **/
      chunktype temp;
      length = st.st_size;
      iterations = std::round( (double) length / 
                     ( (double) (temp.CHUNKSIZE) - chunk_offset - 1 ) );
   }

   virtual raft::kstatus run()
   {
      /** TODO, implement copy version **/
      for( FIFO &port : output )
      {
         if( iterations-- == 0 )
         {
            return( raft::stop );
         }
         auto &chunk( port.allocate< chunktype  >() );
         chunk.start_position = ftell( fp );
         const auto num_read(  
            fread( chunk.buffer, sizeof( char ), chunk.CHUNKSIZE, fp ) );
         chunk.buffer[ num_read ] = '\0';
         chunk.length = num_read;
         port.push( 
            ( iterations - output.count() /* num ports */ ) > 0 ? 
               raft::none : 
               raft::eof );
      }
      return( raft::proceed );
   }

   /** opened in the constructor **/
   FILE *fp = nullptr;
   std::streamsize length;
   std::size_t     iterations;
};
#endif /* END _FILEIO_TCC_ */
