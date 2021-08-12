/**
 * bufferdata.tcc - 
 * @author: Jonathan Beard
 * @version: Fri May 16 13:08:25 2014
 * 
 * Copyright 2020 Jonathan Beard
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
#ifndef RAFTBUFFERDATA_TCC
#define RAFTBUFFERDATA_TCC  1
#include <cstdint>
#include <cstddef>
#include <cstring>
#include <cassert>
#include <thread>
#include <cinttypes>
#include <iostream>
#include <type_traits>
#if defined __APPLE__ || defined __linux
#include <sys/mman.h>
#endif
#include "signalvars.hpp"
#include "pointer.hpp"
#include "ringbuffertypes.hpp"
#include "signal.hpp"
#include "database.tcc"

#include "alloc_traits.tcc"
#include "defs.hpp"


namespace Buffer
{



template < class T,
           Type::RingBufferType B,
           class Enable = void > struct Data{};

/** buffer structure for "heap" storage class **/
template < class T > struct Data< T, 
                                  Type::Heap, 
               typename std::enable_if< inline_alloc< T >::value >::type >
                    : public DataBase< T >
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
        new ( &(this)->read_pt ) Pointer( max_cap );
        new ( &(this)->write_pt) Pointer( max_cap, 1 ); 
        new ( &(this)->read_stats ) Blocked();
        new ( &(this)->write_stats ) Blocked();
        (this)->external_alloc = true;
   }



   Data( const std::size_t max_cap , 
         const std::size_t align = 16 ) : DataBase< T >( max_cap )
   {

#if (defined __linux ) || (defined __APPLE__ )
      int ret_val( 0 );
      ret_val = posix_memalign( (void**)&((this)->store), 
                                 align, 
                                (this)->length_store );
      if( ret_val != 0 )
      {
         std::cerr << "posix_memalign returned error code (" << ret_val << ")";
         std::cerr << " with message: \n" << strerror( ret_val ) << "\n";
         exit( EXIT_FAILURE );
      }
#elif (defined _WIN64 ) || (defined _WIN32) 
      (this)->store = reinterpret_cast< T* >(  _aligned_malloc( (this)->length_store, align ) );
#else
      /** 
       * would use the array allocate, but well...we'd have to 
       * figure out how to free it
       */
      (this)->store = reinterpret_cast< T* >( malloc( (this)->length_store ) );
#endif
      //FIXME - this should be an exception 
      assert( (this)->store != nullptr );
#if (defined __linux ) || (defined __APPLE__ )
      posix_madvise( (this)->store, 
                     (this)->length_store,  
                     POSIX_MADV_SEQUENTIAL );
#endif
      (this)->signal = (Signal*)       calloc( (this)->max_cap,
                                               sizeof( Signal ) );
      if( (this)->signal == nullptr )
      {
         //FIXME - this should be an exception too
         perror( "Failed to allocate signal queue!" );
         exit( EXIT_FAILURE );
      }
      /** allocate read and write pointers **/
      /** TODO, see if there are optimizations to be made with sizing and alignment **/
      new ( &(this)->read_pt ) Pointer( max_cap );
      new ( &(this)->write_pt ) Pointer( max_cap ); 
      new ( &(this)->read_stats ) Blocked();
      new ( &(this)->write_stats ) Blocked();
   }
  
   /**
    * copyFrom - invoke this function when you want to duplicate
    * the FIFO's underlying data structure from one FIFO
    * to the next. You can however no longer use the FIFO
    * "other" unless you are very certain how the implementation
    * works as very bad things might happen.
    * @param other - DataBase< T >*, to be copied
    */
   virtual void copyFrom( DataBase< T > *other )
   {
        if( other->external_alloc )
        {
            //FIXME: throw rafterror that is synchronized
            std::cerr 
                << "FATAL: Attempting to resize a FIFO that is statically alloc'd\n";
            exit( EXIT_FAILURE );
        }
        
        void *ptr( nullptr );
        ptr = new ( &(this)->read_pt ) Pointer( (other->read_pt),   (this)->max_cap );
        ptr = new ( &(this)->write_pt ) Pointer( (other->write_pt), (this)->max_cap );
        UNUSED( ptr );
        (this)->is_valid = other->is_valid;

        /** buffer is already alloc'd, copy **/
        std::memcpy( (void*)(this)->store /* dst */,
                     (void*)other->store  /* src */,
                     other->length_store );
        
        /** copy signal buff **/
        std::memcpy( (void*)(this)->signal /* dst */,
                     (void*)other->signal  /* src */,
                     other->length_signal );
        /** stats objects are still valid, copy the ptrs over **/
        
        (this)->read_stats  = other->read_stats; 
        (this)->write_stats = other->write_stats;
        /** since we might use this as a min size, make persistent **/
        (this)->force_resize = other->force_resize;
        /** everything should be put back together now **/
        (this)->thread_access[ 0 ] = other->thread_access[ 0 ]; 
        (this)->thread_access[ 1 ] = other->thread_access[ 1 ];
   }


   virtual ~Data()
   {
      //FREE USED HERE
      if( ! (this)->external_alloc )
      {
#if (defined _WIN64 ) || (defined _WIN32)
		 _aligned_free( (this)->store );
#else
         free( (this)->store );
#endif
      }
      free( (this)->signal );
   }

}; /** end heap < Line Size **/

/** buffer structure for "heap" storage class > line size **/
template < class T > 
            struct Data< T, 
                         Type::Heap, 
           typename std::enable_if< ext_alloc< T >::value >::type >
                            : public DataBase< T* >
{
   using type_t = T*;
   using ourtype_t = DataBase< type_t >;

   Data( type_t const ptr, 
         const std::size_t max_cap,
         const std::size_t start_position ) : ourtype_t( max_cap )
   {
        assert( ptr != nullptr );
        (this)->store  = reinterpret_cast< type_t* >( ptr );
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
        
        /** allocate read and write pointers **/
        new ( &(this)->read_pt ) Pointer( max_cap );
        new ( &(this)->write_pt ) Pointer( max_cap, 1 ); 
        new ( &(this)->read_stats ) Blocked();
        new ( &(this)->write_stats ) Blocked();
        (this)->external_alloc = true;
   }


   Data( const std::size_t max_cap , 
         const std::size_t align = 16 ) : ourtype_t( max_cap )
   {

#if (defined __linux ) || (defined __APPLE__ )
      const auto ret_val = posix_memalign( (void**)&((this)->store), 
                                           align, 
                                           (this)->length_store );
      if( ret_val != 0 )
      {
         std::cerr << "posix_memalign returned error code (" << ret_val << ")";
         std::cerr << " with message: \n" << strerror( ret_val ) << "\n";
         exit( EXIT_FAILURE );
      }
#elif (defined _WIN64 ) || (defined _WIN32) 
      (this)->store = reinterpret_cast< type_t* >(  _aligned_malloc( (this)->length_store, align ) );
#else
      /** 
       * would use the array allocate, but well...we'd have to 
       * figure out how to free it
       */
      (this)->store = reinterpret_cast< type_t* >( malloc( (this)->length_store ) );
#endif
      //FIXME - this should be an exception 
      assert( (this)->store != nullptr );
      
#if (defined __linux ) || (defined __APPLE__ )
      posix_madvise( (this)->store, 
                     (this)->length_store,  
                     POSIX_MADV_SEQUENTIAL );
#endif
        errno = 0;
        (this)->signal = (Signal*)       calloc( (this)->max_cap,
                                                 sizeof( Signal ) );
        if( (this)->signal == nullptr )
        {
           perror( "Failed to allocate signal queue!" );
           exit( EXIT_FAILURE );
        }
        /** allocate read and write pointers **/
        new ( &(this)->read_pt ) Pointer( max_cap );
        new ( &(this)->write_pt) Pointer( max_cap ); 
        new ( &(this)->read_stats ) Blocked();
        new ( &(this)->write_stats ) Blocked();
   }
   
   virtual void copyFrom( ourtype_t *other )
   {
        if( other->external_alloc )
        {
            //FIXME: throw rafterror that is synchronized
            std::cerr << 
                "FATAL: Attempting to resize a FIFO that is statically alloc'd\n";
            exit( EXIT_FAILURE );
        }
        new ( &(this)->read_pt  ) Pointer( (other->read_pt),   
                                           (this)->max_cap );
        new ( &(this)->write_pt ) Pointer( (other->write_pt), 
                                           (this)->max_cap );
        (this)->is_valid = other->is_valid;

        /** buffer is already alloc'd, copy **/
        std::memcpy( (void*)(this)->store /* dst */,
                     (void*)other->store  /* src */,
                     other->length_store );
        /** copy signal buff **/
        std::memcpy( (void*)(this)->signal /* dst */,
                     (void*)other->signal  /* src */,
                     other->length_signal );
        //copy over block stats objects
        (this)->read_stats  = other->read_stats; 
        (this)->write_stats = other->write_stats;
        
        (this)->thread_access[ 0 ] = other->thread_access[ 0 ]; 
        (this)->thread_access[ 1 ] = other->thread_access[ 1 ];
        /** everything should be put back together now **/
   }


   virtual ~Data()
   {
      //FREE USED HERE
      if( ! (this)->external_alloc )
      {
#if (defined _WIN64 ) || (defined _WIN32) 
		 _aligned_free( (this)->store );
#else
         free( (this)->store );
#endif
      }
      free( (this)->signal );
   }

}; /** end heap > Line Size **/



} //end namespace Buffer
#endif /* END RAFTBUFFERDATA_TCC */
