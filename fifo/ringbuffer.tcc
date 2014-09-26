/**
 * ringbuffer.tcc - 
 * @author: Jonathan Beard
 * @version: Wed Apr 16 14:18:43 2014
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
 * 
 * Notes:  When using monitoring, the cycle counter is the most accurate for
 * Linux / Unix platforms.  It is really not suited to OS X's mach_absolute_time()
 * function since it is so slow relative to the movement of data, the 
 * results returned for high throughput systems are simply not accurate
 * on that platform.
 */
#ifndef _RINGBUFFER_TCC_
#define _RINGBUFFER_TCC_  1

#include <array>
#include <cstdlib>
#include <thread>
#include <cstring>
#include <cstdint>
#include <vector>
#include <iostream>
#include <fstream>
#include <utility>
#include <cstddef>

#include "ringbufferbase.tcc"
#include "ringbuffertypes.hpp"
#include "SystemClock.tcc"
#include "sample.tcc"
#include "meansampletype.tcc"
#include "arrivalratesampletype.tcc"
#include "departureratesampletype.tcc"

extern Clock *system_clock;


template < class T, 
           Type::RingBufferType type = Type::Heap, 
           bool monitor = false >  class RingBuffer : 
               public RingBufferBase< T, type >
{
public:
   /**
    * RingBuffer - default constructor, initializes basic
    * data structures.
    */
   RingBuffer( const std::size_t n, const std::size_t align = 16 ) : 
      RingBufferBase< T, type >()
   {
      (this)->data = new Buffer::Data<T, type >( n, align );
   }

   virtual ~RingBuffer()
   {
      delete( (this)->data );
   }

   /**
    * make_new_fifo - builder function to dynamically
    * allocate FIFO's at the time of execution.  The
    * first two parameters are self explanatory.  The
    * data ptr is a data struct that is dependent on the
    * type of FIFO being built.  In there really is no
    * data necessary so it is expacted to be set to nullptr
    * @param   n_items - std::size_t
    * @param   align   - memory alignment
    * @return  FIFO*
    */
   static FIFO* make_new_fifo( std::size_t n_items,
                               std::size_t align,
                               void *data )
   {
      assert( data == nullptr );
      return( new RingBuffer< T, Type::Heap, false >( n_items, align ) ); 
   }

};

template< class T, 
          Type::RingBufferType type > class RingBufferBaseMonitor : 
            public RingBufferBase< T, type >
{
public:
   RingBufferBaseMonitor( const std::size_t n,
                          const std::size_t align ) : 
            RingBufferBase< T, type >(),
            monitor( nullptr ),
            term( false )
   {
      (this)->data = new Buffer::Data<T, 
                                      Type::Heap >( n, align );

      /** add monitor types immediately after construction **/
      sample_master.registerSample( new MeanSampleType< T, type >() );
      sample_master.registerSample( new ArrivalRateSampleType< T, type >() );
      sample_master.registerSample( new DepartureRateSampleType< T, type > () );
      (this)->monitor = new std::thread( Sample< T, type >::run, 
                                         std::ref( *(this)      /** buffer **/ ),
                                         std::ref( (this)->term /** term bool **/ ),
                                         std::ref( (this)->sample_master ) );

   }

   void  monitor_off()
   {
      (this)->term = true;
   }

   virtual ~RingBufferBaseMonitor()
   {
      (this)->term = true;
      monitor->join();
      delete( monitor );
      monitor = nullptr;
      delete( (this)->data );
   }

   std::ostream&
   printQueueData( std::ostream &stream )
   {
      stream << sample_master.printAllData( '\n' );
      return( stream );
   }
protected:
   std::thread       *monitor;
   volatile bool      term;
   Sample< T, type >  sample_master;
};

template< class T > class RingBuffer< T, 
                                      Type::Heap,
                                      true /* monitor */ > :
      public RingBufferBaseMonitor< T, Type::Heap >
{
public:
   /**
    * RingBuffer - default constructor, initializes basic
    * data structures.
    */
   RingBuffer( const std::size_t n, 
               const std::size_t align = 16) : 
                  RingBufferBaseMonitor< T, Type::Heap >( n, align)
   {
      /** nothing really to do **/
   }
   
   virtual ~RingBuffer()
   {
      /** nothing really to do **/
   }
   
   static FIFO* make_new_fifo( std::size_t n_items,
                               std::size_t align,
                               void *data )
   {
      assert( data == nullptr );
      return( new RingBuffer< T, Type::Heap, true >( n_items, align ) ); 
   }
};

/** specialization for dummy one **/
template< class T > class RingBuffer< T, 
                                      Type::Infinite,
                                      true /* monitor */ > :
      public RingBufferBaseMonitor< T, Type::Infinite >
{
public:
   /**
    * RingBuffer - default constructor, initializes basic
    * data structures.
    */
   RingBuffer( const std::size_t n, const std::size_t align = 16 ) : 
      RingBufferBaseMonitor< T, Type::Infinite >( 1, align )
   {
   }
   virtual ~RingBuffer()
   {
      /** nothing really to do **/
   }

   static FIFO* make_new_fifo( std::size_t n_items,
                               std::size_t align,
                               void *data )
   {
      assert( data == nullptr );
      return( new RingBuffer< T, Type::Infinite, true >( n_items, align ) ); 
   }
};

/** specialization for dummy with no instrumentation **/
template < class T > class RingBuffer < T,
                                        Type::Infinite,
                                        false > : 
               public RingBufferBase< T, Type::Infinite >
{
public:
   /**
    * RingBuffer - default constructor, initializes basic
    * data structures.
    */
   RingBuffer( const std::size_t n, const std::size_t align = 16 ) : 
      RingBufferBase< T, Type::Infinite >()
   {
      (this)->data = new Buffer::Data<T, Type::Heap >( 1, 16 );
   }

   virtual ~RingBuffer()
   {
      delete( (this)->data );
   }

   /**
    * make_new_fifo - builder function to dynamically
    * allocate FIFO's at the time of execution.  The
    * first two parameters are self explanatory.  The
    * data ptr is a data struct that is dependent on the
    * type of FIFO being built.  In there really is no
    * data necessary so it is expacted to be set to nullptr
    * @param   n_items - std::size_t
    * @param   align   - memory alignment
    * @return  FIFO*
    */
   static FIFO* make_new_fifo( std::size_t n_items,
                               std::size_t align,
                               void *data )
   {
      assert( data == nullptr );
      return( new RingBuffer< T, Type::Infinite, false >( n_items, align ) ); 
   }

};


/** 
 * SharedMemory 
 */
template< class T > class RingBuffer< T, 
                                      Type::SharedMemory, 
                                      false > :
                            public RingBufferBase< T, Type::SharedMemory >
{
public:
   RingBuffer( const std::size_t      nitems,
               const std::string key,
               Direction         dir,
               const std::size_t      alignment = 16 ) : 
               RingBufferBase< T, Type::SharedMemory >(),
                                              shm_key( key )
   {
      (this)->data = 
         new Buffer::Data< T, 
                           Type::SharedMemory >( nitems, key, dir, alignment );
      assert( (this)->data != nullptr );
   }

   virtual ~RingBuffer()
   {
      delete( (this)->data );      
   }
  
   struct Data
   {
      const std::string key;
      Direction   dir;
   };

   /**
    * make_new_fifo - builder function to dynamically
    * allocate FIFO's at the time of execution.  The
    * first two parameters are self explanatory.  The
    * data ptr is a data struct that is dependent on the
    * type of FIFO being built.  In there really is no
    * data necessary so it is expacted to be set to nullptr
    * @param   n_items - std::size_t
    * @param   align   - memory alignment
    * @return  FIFO*
    */
   static FIFO* make_new_fifo( std::size_t n_items,
                               std::size_t align,
                               void *data )
   {
      auto *data_ptr( reinterpret_cast< Data* >( data ) );
      return( new RingBuffer< T, Type::SharedMemory, false >( n_items, 
                                                              data_ptr->key,
                                                              data_ptr->dir,
                                                              align ) ); 
   }

protected:
   const  std::string shm_key;
};


/**
 * TCP w/ multiplexing
 */
template <class T> class RingBuffer< T,
                                     Type::TCP,
                                     false /* no monitoring yet */ > :
                                       public RingBufferBase< T, Type::Heap >
{
public:
   RingBuffer( const std::size_t      nitems,
               const std::string dns_name,
               Direction         dir,
               const std::size_t      alignment = 16 ) : 
                  RingBufferBase< T, 
                                  Type::Heap >()
   {
      //TODO, fill in stuff here
   }

   virtual ~RingBuffer()
   {

   }
   
   struct Data
   {
      Direction   dir;
      std::string dns_name;
   };

   static FIFO* make_new_fifo( const std::size_t n,
                               const std::size_t align,
                               void *data )
   {
      auto *cast_data( 
         reinterpret_cast< RingBuffer< T, Type::TCP, false >::Data* >( data ) );

      return( new RingBuffer< T, Type::TCP, false >(n /** n_items **/,
                                                    cast_data->dns_name,
                                                    cast_data->dir,
                                                    align ) );
   }

protected:
};
#endif /* END _RINGBUFFER_TCC_ */
