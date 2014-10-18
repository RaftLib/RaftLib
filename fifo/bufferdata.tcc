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
#include <cstring>
#include <cassert>
#include <thread>
#include "shm.hpp"
#include "signalvars.hpp"
#include "pointer.hpp"
#include "ringbuffertypes.hpp"


namespace Buffer
{

/**
 * Element - simple struct that wraps each
 * data element.  This makes modifying the 
 * structure much easier in the future.
 */
template < class X > struct Element
{
   /** default constructor **/
   Element()
   {
   }
   /**
    * Element - copy constructor, the type
    * X must have a defined assignment operator.
    * This is simple for primitive types, but
    * probably must be defined for objects,
    * structs and more complex types.
    */
   Element( const Element< X > &other )
   {
      (this)->item   = other.item;
   }

   X item;
};

/**
 * Signal - just like the Element struct,
 * the signal is wrapped as well.
 */
struct Signal
{
   Signal() : sig( raft::none )
   {
   }

   Signal( const Signal &other )
   {
      (this)->sig = other.sig;
   }

   /**
    * operator =, enable taking a signal rhs
    * and assign it to a Signal struct type,
    * makes programming a bit easier, especially
    * if we change names of internal members
    */
   Signal& operator = ( raft::signal signal )
   {
      (this)->sig = signal;
      return( (*this) );
   }
   
   /**
    * operator =, enable taking a signal rhs
    * and assign it to a Signal struct type,
    * makes programming a bit easier, especially
    * if we change names of internal members
    */
   Signal& operator = ( raft::signal &signal )
   {
      (this)->sig = signal;
      return( (*this) );
   }
   
   /** 
    * allow casting struct back to simple
    * signal type
    */
   operator raft::signal()
   {
      return( (this)->sig );
   }
   raft::signal sig;
};


/**
 * DataBase - not quite the best name since we 
 * conjure up a relational database, but it is
 * literally the base for the Data structs below.
 */
template < class T > struct DataBase 
{
   DataBase( const size_t max_cap ) : read_pt ( nullptr ),
                                      write_pt( nullptr ),
                                      max_cap ( max_cap ),
                                      store   ( nullptr ),
                                      signal  ( nullptr )
   {

      length_store   = ( sizeof( Element< T > ) * max_cap ); 
      length_signal  = ( sizeof( Signal ) * max_cap );
   }

   /** 
    * copyFrom - implement in all sub-structs to 
    * copy the buffer.  Might need to reinterpret
    * cast the other object or other type of cast
    * in order to get all the data members you wish
    * to copy.
    * @param   other - struct to be copied
    */
   virtual void copyFrom( DataBase< T > *other ) = 0;

   Pointer           *read_pt;
   Pointer           *write_pt;
   size_t             max_cap;
   /** 
    * allocating these as structs gives a bit
    * more flexibility later in what to pass
    * along with the queue.  It'll be more 
    * efficient copy wise to pass extra items
    * in the signal, but conceivably there could
    * be a case for adding items in the store
    * as well.
    */
   Element< T >      *store;
   Signal            *signal;
   size_t             length_store;
   size_t             length_signal;
};

template < class T, 
           Type::RingBufferType B = Type::Heap, 
           size_t SIZE = 0 > struct Data : public DataBase< T >
{


   Data( size_t max_cap , const size_t align = 16 ) : DataBase< T >( max_cap )
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
      delete( (this)->read_pt );
      (this)->read_pt = new Pointer( (other->read_pt),   (this)->max_cap );
      delete( (this)->write_pt );
      (this)->write_pt = new Pointer( (other->write_pt), (this)->max_cap );

      /** buffer is already alloc'd, copy **/
      std::memmove( (void*)(this)->store /* dst */,
                   (void*)other->store  /* src */,
                   other->length_store );
      /** copy signal buff **/
      std::memmove( (void*)(this)->signal /* dst */,
                   (void*)other->signal  /* src */,
                   other->length_signal );
      /** everything should be put back together now **/
   }


   ~Data()
   {
      //DELETE USED HERE
      delete( (this)->read_pt );
      delete( (this)->write_pt );
      //FREE USED HERE
      free( (this)->store );
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
    * @param   shm_key, const std::string key for opening the SHM, must be same for both ends of the queue
    * @param   dir,     Direction enum for letting this queue know which side we're allocating
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
               int timeout( 1000 );
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
      int32_t producer;
      int32_t consumer;;
   };

   volatile Cookie         *cookie;

   /** process local key copies **/
   const std::string store_key; 
   const std::string signal_key;  
   const std::string ptr_key; 
};
}
#endif /* END _BUFFERDATA_TCC_ */
