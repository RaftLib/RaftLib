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
#include "systeminfo.hpp"
#include "ringbuffer.tcc"
#include "SystemClock.tcc"


Clock *system_clock = new SystemClock< System >( 1 /* assigned core */ );

enum SearchAlgorithm 
{ 
   RabinKarp,
   KnuthMorrisPratt,
   Automata
};

#define CHUNKSIZE 16536

struct Chunk
{
   Chunk() : start_position( 0 ),
             length( 0 )
   {
      std::memset( chunk, '\0', CHUNKSIZE ); 
   }

   Chunk( const Chunk &other ) : start_position( other.start_position ),
                                 length( other.length )
   {
      std::strncpy( chunk, other.chunk, CHUNKSIZE );
   }

   void operator = ( const Chunk &other )
   {
      start_position = other.start_position;
      length         = other.length;
      std::strncpy( chunk, other.chunk, CHUNKSIZE );
   }

   size_t start_position;
   size_t length;
   char   chunk[ CHUNKSIZE ];
};

typedef size_t Hit;

#if MONITOR==1
typedef RingBuffer< Chunk, 
                    RingBufferType::Infinite,
                    true > InputBuffer;
typedef RingBuffer< Hit,
                    RingBufferType::Infinite,
                    true > OutputBuffer;
#else
typedef RingBuffer< Chunk > InputBuffer;
typedef RingBuffer< Hit   > OutputBuffer;
#endif

template < size_t THREADS, 
           size_t BUFFSIZE = 100 > class Search
{
public:
   Search()          = delete;
   virtual ~Search() = delete;
   
   template< SearchAlgorithm algorithm >
   static void search( const std::string           filename,
                       const std::string           search_term,
                       std::vector< Hit >  &hits )
   {


      std::array< std::thread*,  THREADS + 1 > thread_pool;
      std::array< InputBuffer*,  THREADS > input_buffer;
      std::array< OutputBuffer*, THREADS > output_buffer;
      
      /** allocate buffers **/
      for( size_t buff_id( 0 ); buff_id < THREADS; buff_id++ )
      {
         input_buffer  [  buff_id ] = new InputBuffer( BUFFSIZE );
         output_buffer [ buff_id  ] = new OutputBuffer( BUFFSIZE );
      }
      
      /** allocate consumer thread **/
      thread_pool[ THREADS ] = new std::thread( consumer_function,
                                                std::ref( output_buffer ),
                                                std::ref( hits ) );

      /** get input file **/
      std::ifstream file_input( filename, std::ifstream::binary );
      if( ! file_input.is_open() )
      {
         std::cerr << "Failed to open input file: " << filename << "\n";
         exit( EXIT_FAILURE );
      }
     
      /** declare iterations, needed to send stop signal **/
      int64_t  iterations( 0 );
      /**
       * get longest & smallest search term 
       */
      const uint64_t search_term_length( search_term.length() );

      /**
       * get file length 
       */
      file_input.seekg( 0, file_input.end );
      const auto file_length( file_input.tellg() );
      file_input.seekg( 0, file_input.beg );

      /** declare the worker thread function **/
      std::function< void( InputBuffer*, OutputBuffer* ) > worker_function;
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
               int64_t t( 0 );
               for( size_t i( 0 ); i < length; i++ )
               {
                  t = ( ( t * d ) + line[ i ] ) % q;
               }
               return( t );
            };
            
            /**
             * h - max radix power to subtract off in rolling hash
             */
            uint64_t h( 1 );
            /**
             * p - pattern hash value, only computed once and read
             * only after that
             */
            int64_t p(  hash_function( search_term, search_term_length ) );
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
      
      
      for( size_t thread_id( 0 ); thread_id < THREADS; thread_id++ )
      {
         thread_pool[ thread_id ] = new std::thread( worker_function,
                                                     input_buffer[  thread_id ],
                                                     output_buffer[ thread_id ] );
      }
   
      size_t output_stream( 0 );
#if FAKEDATA == 1
      size_t fake_counter( 1 * (2^30) );
#endif
      while( file_input.good() )
      {
         /** input stream with reference to the worker thread **/
         auto *input_stream( input_buffer[ output_stream ] );
         Chunk &chunk( input_stream->allocate() );
         chunk.start_position = file_input.tellg();
         file_input.read( chunk.chunk, CHUNKSIZE );
         chunk.length = ( size_t )file_input.gcount();
#if FAKEDATA == 1         
         fake_counter -= chunk.length;
         if( fake_counter <= 0 )
         {
            break;
         }
#endif         
         input_stream->push( /*RBSignal::NONE*/); 
         if( iterations )
         {
            file_input.seekg( 
               (size_t) file_input.tellg() - search_term_length - 1 );
         }
         output_stream = ( output_stream + 1 ) % THREADS;
      }

      /** until I fix the asynchronous signalling, send dummy **/ 
      for( InputBuffer *buff : input_buffer )
      {
         Chunk &chunk( buff->allocate() );
         chunk.length = 0;
         buff->push( RBSignal::TERM );
      }
      
      /** end of file, wait for results **/
      for( std::thread *th : thread_pool )
      {
         th->join();
         delete( th );
         th = nullptr;
      }
#if MONITOR==1
      std::stringstream ss;
      ss << "/project/mercury/svardata/";
      std::string filehead( SystemInfo::getSystemProperty( Trait::NodeName ) );
      ss << filehead << "_search_" << QUEUETYPE << "_" << THREADS << ".csv";
      std::string m_filename( ss.str() );
      std::ofstream monitorfile( m_filename, std::fstream::app | std::fstream::out );
      if( ! monitorfile.is_open() )
      {
         std::cerr << "Failed to open file!\n";
         exit( EXIT_FAILURE );
      }
      std::string traits;
      {
         std::stringstream trait_stream;
         const auto num_traits( SystemInfo::getNumTraits() );
         for( auto index( 0 ); index < num_traits; index++ )
         {
            trait_stream << SystemInfo::getSystemProperty( (Trait) index ) << ",";
         }
         traits = trait_stream.str();
      }
#endif
      for( InputBuffer *buff : input_buffer )
      {
#if MONITOR==1
         auto &monitor_data( buff->getQueueData() );
         monitorfile << traits;
         Monitor::QueueData::print( monitor_data,
                                    Monitor::QueueData::Bytes,
                                    monitorfile,
                                    true );
         monitorfile << "\n";
#endif
         delete( buff );
         buff = nullptr;
      }

      for( OutputBuffer *buff : output_buffer )
      {
#if MONITOR==1
         auto &monitor_data( buff->getQueueData() );
         monitorfile << traits;
         Monitor::QueueData::print( monitor_data,
                                    Monitor::QueueData::Bytes,
                                    monitorfile,
                                    true );
         monitorfile << "\n";
#endif
         delete( buff );
         buff = nullptr;
      }
      
      file_input.close();
#if MONITOR
      monitorfile.close();
#endif
   }
private:

   static void worker_function_base( InputBuffer *input, 
                                     OutputBuffer *output,
                                     std::function< 
                                     void ( Chunk&, 
                                            std::vector< Hit >& ) > 
                                             search_function )
   {
      assert( input != nullptr );
      assert( output != nullptr );
      std::vector< Hit > local_hits;
      RBSignal signal( RBSignal::NONE );
      while( true  ) 
      {
         if( input->size() > 0 )
         {
            auto &chunk( input->peek( &signal ) );
            if( signal == RBSignal::TERM )
            {
               /** 
                * TODO, gotta redo the asynchronous signalling so in the mean time,
                * this will work.
                */
               input->recycle();
               auto &temp( output->allocate() );
               temp = 0;
               output->push( RBSignal::TERM );
               return;
            }
            search_function( chunk, local_hits );
            if( local_hits.size() > 0 )
            {
               output->insert( local_hits.begin(), local_hits.end() );
               local_hits.clear();
            }
            input->recycle();
         }
      }
   }

   /** 
    * TODO, add verification kernel...spurious hits are unlikely but with 
    * a bit enough file they can happen.
    */

   static void consumer_function( std::array< OutputBuffer*, THREADS > &input,
                                  std::vector< Hit >                &hits )
   {
      size_t term_sig_count( 0 );
      Hit hit;
      RBSignal signal( RBSignal::NONE );
      while( term_sig_count < THREADS )
      {
         for( auto *buff : input )
         {
            if( buff->size() > 0 )
            {
               buff->pop( hit, &signal );
               if( signal == RBSignal::TERM )
               {
                  term_sig_count++;
               }
               else
               {
                  hits.push_back( hit );
               }
            }
         }
      }
      return;
   }
};
#endif /* END _SEARCH_TCC_ */
