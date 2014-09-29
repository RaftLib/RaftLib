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
#include <type>
#include <fstream>
#include <cstddef>
#include <raft>

template < std::size_t size > struct filechunk
{
   char              buffer[ size ]; 
   const std::size_t CHUNKSIZE = size;
   std::size_t       BUFFERSIZE;
};

template < struct filechunk, bool copy > filereader : public Kernel
{
public:
   filereader( const std::string inputfile, const std::size_t output_ports )
   {
      const std::string prefix( "output_" );
      for( auto index( 0 ); index < output_ports; index++ )
      {
         /** add a port for each index var, all named "output_#" **/
         output.addPort< filechunk >( prefix + std::to_string( index ) );
      }
      /** TODO, rewrite with C functions...takes a bit longer to write so I'm doing this for now **/
      std::ifstream file_input( filename, std::ifstream::binary );
      if( ! file_input.is_open() )
      {
         std::cerr << "Failed to open input file: " << filename << "\n";
         exit( EXIT_FAILURE );
      }
      /** get length **/
      file_input.seekg( 0, file_input.end );
      length = file_input.tellg();
      file_input.seekg( 0, file_input.beg );
   }

   virtual raft::kstatus run()
   {
       
   }

   /** opened in the constructor **/
   std::ifstream   file_input;
   std::streamsize length;
};
#endif /* END _FILEIO_TCC_ */
