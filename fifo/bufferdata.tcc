/**
 * bufferdata.tcc - 
 * @author: Jonathan Beard
 * @version: Fri May 16 13:08:25 2014
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
#ifndef _BUFFERDATA_TCC_
#define _BUFFERDATA_TCC_  1
#include <cstdint>
#include <cstddef>
#include <cstring>
#include <cassert>
#include <thread>
#include <cinttypes>
#include <iostream>

#include "shm.hpp"
#include "signalvars.hpp"
#include "pointer.hpp"
#include "ringbuffertypes.hpp"

#include "signal.hpp"

#include "database.tcc"

namespace Buffer
{


/** buffer structure for "heap" storage class **/
template < class T, 
           Type::RingBufferType B = Type::Heap, 
           std::size_t SIZE = 0 > struct Data : public DataBase< T >
{

   Data( T  * const ptr, 
         const std::size_t max_cap,
         const std::size_t start_position ) : DataBase< T >( max_cap )
   {
      assert( ptr != nullptr );
      (this)->store  = ptr;
      (this)->signal = (Signal*)       calloc( 1,
                                               sizeof( Signal ) );
      if( (this)->signal == nullptr )
      {
         perror( "Failed to allocate signal queue!" );
         exit( EXIT_FAILURE );
      }
      /** set index to be start_position **/
      (this)->signal[ 0 ].index  = start_position; 
      /** allocate read and write pointers **/
      (this)->read_pt   = new Pointer( max_cap );
      (this)->write_pt  = new Pointer( max_cap, 1 ); 
      
      (this)->external_alloc = true;
   }


   Data( const std::size_t max_cap , 
         const std::size_t align = 16 ) : DataBase< T >( max_cap )
   {
      int ret_val( posix_memalign( (void**)&((this)->store), 
                                   align, 
                                   (this)->length_store ) );
      if( ret_val != 0 )
      {
         std::cerr << "posix_memalign returned error code (" << ret_val << ")";
         std::cerr << " with message: \n" << strerror( ret_val ) << "\n";
         exit( EXIT_FAILURE );
      }
      
      //posix_madvise( (this)->store, (this)->length_store,  POSIX_MADV_WILLNEED );

      errno = 0;
      (this)->signal = (Signal*)       calloc( (this)->max_cap,
                                               sizeof( Signal ) );
      if( (this)->signal == nullptr )
      {
         perror( "Failed to allocate signal queue!" );
         exit( EXIT_FAILURE );
      }
      /** allocate read and write pointers **/
      /** TODO, see if there are optimizations to be made with sizing and alignment **/
      (this)->read_pt   = new Pointer( max_cap );
      (this)->write_pt  = new Pointer( max_cap ); 
   }
   
   virtual void copyFrom( DataBase< T > *other )
   {
      //TODO, figure something better out for here 
      if( other->external_alloc )
      {
         assert( false );
      }
      delete( (this)->read_pt );
      (this)->read_pt = new Pointer( (other->read_pt),   (this)->max_cap );
      delete( (this)->write_pt );
      (this)->write_pt = new Pointer( (other->write_pt), (this)->max_cap );

      (this)->src_kernel = other->src_kernel;
      (this)->dst_kernel = other->dst_kernel;
      
      (this)->is_valid = other->is_valid;

      /** buffer is already alloc'd, copy **/
      std::memcpy( (void*)(this)->store /* dst */,
                   (void*)other->store  /* src */,
                   other->length_store );
      /** copy signal buff **/
      std::memcpy( (void*)(this)->signal /* dst */,
                   (void*)other->signal  /* src */,
                   other->length_signal );
      /** everything should be put back together now **/
   }


   virtual ~Data()
   {
      //DELETE USED HERE
      delete( (this)->read_pt );
      delete( (this)->write_pt );
      
      //FREE USED HERE
      if( ! (this)->external_alloc )
      {
         free( (this)->store );
      }
      free( (this)->signal );
   }

}; /** end heap **/

template < class T > struct Data< T, Type::SharedMemory > : 
   public DataBase< T > 
{
   /**
    * Data - Constructor for SHM based ringbuffer.  Allocates store, signal and 
    * ptr structures in separate SHM segments.  Could have been a single one but
    * makes reading the ptr arithmatic a bit more difficult.  TODO, reevaluate
    * if performance wise that might be a good idea, also decide how best to 
    * align data elements.
    * @param   max_cap, size_t with number of items to allocate queue for
    * @param   shm_key, const std::string key for opening the SHM, 
                        must be same for both ends of the queue
    * @param   dir,     Direction enum for letting this queue know 
                        which side we're allocating
    * @param   alignment, size_t with alignment NOTE: haven't implemented yet
    */
   Data( size_t max_cap, 
         const std::string shm_key,
         Direction dir,
         const size_t alignment ) : DataBase< T >( max_cap ),
                                    store_key( shm_key + "_store" ),
                                    signal_key( shm_key + "_key" ),
                                    ptr_key( shm_key + "_ptr" )
   {
      /** now work through opening SHM **/
      switch( dir )
      {
         case( Direction::Producer ):
         {
            auto alloc_with_error = 
            [&]( void **ptr, const size_t length, const char *key )
            {
               try
               {
                  *ptr = SHM::Init( key, length );
               }catch( bad_shm_alloc &ex )
               {
                  std::cerr << 
                  "Bad SHM allocate for key (" << 
                     key << ") with length (" << length << ")\n";
                  std::cerr << "Message: " << ex.what() << ", exiting.\n";
                  exit( EXIT_FAILURE );
               }
               assert( *ptr != nullptr );
            };
            alloc_with_error( (void**)&(this)->store, 
                              (this)->length_store, 
                              store_key.c_str() );
            alloc_with_error( (void**)&(this)->signal, 
                              (this)->length_signal, 
                              signal_key.c_str() );
            alloc_with_error( (void**)&(this)->read_pt, 
                              (sizeof( Pointer ) * 2 ) + sizeof( Cookie ), 
                              ptr_key.c_str() );

            (this)->write_pt = &(this)->read_pt[ 1 ];
            
            Pointer temp( max_cap );
            std::memcpy( (this)->read_pt,  &temp, sizeof( Pointer ) );
            std::memcpy( (this)->write_pt, &temp, sizeof( Pointer ) );
            
            (this)->cookie   = (Cookie*) &(this)->read_pt[ 2 ];
            (this)->cookie->producer = 0x1337;
            while( (this)->cookie->consumer != 0x1337 )
            {
               std::this_thread::yield();
            }
         }
         break;
         case( Direction::Consumer ):
         {
            auto retry_func = [&]( void **ptr, const char *str )
            {
               std::string error_copy;
               std::int32_t timeout( 1000 );
               while( timeout-- )
               {
                  try
                  {
                     *ptr = SHM::Open( str );
                  }
                  catch( bad_shm_alloc &ex )
                  {
                     //do nothing
                     timeout--;
                     error_copy = ex.what();
                     std::this_thread::yield();
                     continue;
                  }
                  goto SUCCESS;
               }
               /** timeout reached **/
               std::cerr << "Failed to open shared memory for \"" << 
                  str << "\", exiting!!\n";
               std::cerr << "Error message: " << error_copy << "\n";
               exit( EXIT_FAILURE );
               SUCCESS:;
            };
            retry_func( (void**) &(this)->store,  store_key.c_str() );
            retry_func( (void**) &(this)->signal, signal_key.c_str() );
            retry_func( (void**) &(this)->read_pt, ptr_key.c_str() );
            
            assert( (this)->store != nullptr );
            assert( (this)->signal != nullptr );
            assert( (this)->read_pt   != nullptr );
            
            /** fix write_pt **/
            (this)->write_pt  = &(this)->read_pt[ 1 ];
            assert( (this)->write_pt  != nullptr );
            (this)->cookie    = (Cookie*)&(this)->read_pt[ 2 ]; 
            
            Pointer temp( max_cap );
            std::memcpy( (this)->read_pt,  &temp, sizeof( Pointer ) );
            std::memcpy( (this)->write_pt, &temp, sizeof( Pointer ) );
            
            (this)->cookie->consumer = 0x1337;
            while( (this)->cookie->producer != 0x1337 )
            {
               std::this_thread::yield();
            }
         }
         break;
         default:
         {
            //TODO, add signal handler to cleanup
            std::cerr << "Invalid direction, exiting\n";
            exit( EXIT_FAILURE );
         }
      }
      /** should be all set now **/
   }

   virtual void copyFrom( DataBase< T > *other )
   {
      assert( false );
      /** TODO, implement me **/
   }

   ~Data()
   {
      /** three segments of SHM to close **/
      SHM::Close( store_key.c_str(), 
                  (void*) (this)->store, 
                  (this)->length_store,
                  false,
                  true );
      SHM::Close( signal_key.c_str(),
                  (void*) (this)->signal,
                  (this)->length_signal,
                  false,
                  true );
      SHM::Close( ptr_key.c_str(),   
                  (void*) (this)->read_pt, 
                  (sizeof( Pointer ) * 2) + sizeof( Cookie ),
                  false,
                  true );
   }
   struct Cookie
   {
      std::int32_t producer;
      std::int32_t consumer;;
   };

   volatile Cookie         *cookie;

   /** process local key copies **/
   const std::string store_key; 
   const std::string signal_key;  
   const std::string ptr_key; 
};
}
#endif /* END _BUFFERDATA_TCC_ */
