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

namespace raft{

template < std::size_t size = 65536 > struct filechunk
{
   filechunk() = default;

   filechunk( const filechunk< size > &other )
   {
      std::memcpy( buffer, other.buffer, other.length + 1 );
      start_position = other.start_position;
      length = other.length;
   }

   char              buffer[ size ]; 
   std::size_t       start_position;
   std::size_t       length;

   constexpr static std::size_t getChunkSize()
   {
      return( size );
   }

   friend std::ostream& operator <<( std::ostream &output, filechunk< size > &c )
   {
      output << c.buffer;
      return( output );
   }
};

template < class chunktype = filechunk< 65536 >, 
           bool copy = false > class filereader : public raft::kernel
{
public:
   filereader( const std::string inputfile, 
               const std::size_t n_output_ports = 1,
               const std::size_t chunk_offset = 0 )
   {
      for( auto index( 0 ); index < n_output_ports; index++ )
      {
         /** add a port for each index var, all named "output_#" **/
         output.addPort< chunktype  >( std::to_string( index ) );
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
      length = st.st_size;
      iterations = std::round( (double) length / 
                     ( (double) (chunktype::getChunkSize()) - chunk_offset - 1 ) );
   }

   virtual raft::kstatus run()
   {  
      for( auto &port : output )
      {
         auto &chunk( port.template allocate< chunktype  >() );
         chunk.start_position = ftell( fp );
         const auto chunksize( chunktype::getChunkSize() );
         const auto num_read(  
            fread( chunk.buffer, sizeof( char ), chunksize , fp ) );
         chunk.buffer[ num_read ] = '\0';
         chunk.length = num_read;
         port.send( 
            ( iterations - output.count() /* num ports */ ) > 0 ? 
               raft::none : 
               raft::eof );
         
         if( --iterations <= 0 )
         {  
            return( raft::stop );
         }
      }
      return( raft::proceed );
   }

   /** opened in the constructor **/
   FILE           *fp         = nullptr;
   std::streamsize length     = 0;
   std::int64_t    iterations = 0;
};

} /* end namespace raft */
#endif /* END _FILEIO_TCC_ */
