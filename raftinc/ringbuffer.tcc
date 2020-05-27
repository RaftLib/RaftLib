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
 */
#ifndef RAFTRINGBUFFER_TCC
#define RAFTRINGBUFFER_TCC 1

#include <array>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <thread>
#include <utility>
#include <vector>

#include "fifoabstract.tcc"
#include "ringbufferbase.tcc"
#include "ringbuffertypes.hpp"



//#include "sample.tcc"
//#include "meansampletype.tcc"
//#include "arrivalratesampletype.tcc"
//#include "departureratesampletype.tcc"


/**
 * RingBuffer, default type is a heap.  This version
 * has no "monitor" thread, but does have the ability
 * to query queue size which can be quite useful for
 * some monitoring tasks.
 */
template <class T, Type::RingBufferType type = Type::Heap, bool monitor = false>
class RingBuffer : public RingBufferBase<T, type>
{
public:
    /**
     * RingBuffer - default constructor, initializes basic
     * data structures.
     */
    RingBuffer(const std::size_t n, const std::size_t align = 16)
        : RingBufferBase<T, type>()
    {
        assert(n != 0);
        (this)->datamanager.set( new Buffer::Data<T, type>(n, align) );
        /** set call-backs to structures inside buffer **/
        (this)->init();
    }

    /**
     * RingBuffer - default constructor, initializes basic
     * data structures.
     */
    RingBuffer(void* const ptr, const std::size_t length,
        const std::size_t start_position)
        : RingBufferBase<T, type>()
    {
        assert(length != 0);
        T* ptrcast = reinterpret_cast<T*>(ptr);
        (this)->datamanager.set(
            new Buffer::Data<T, type>(ptrcast, length, start_position));
        /** set call-backs to structures inside buffer **/
        (this)->init();
    }

    virtual ~RingBuffer()
    {
        /** TODO, might need to get a lock here **/
        /** might have bad behavior if we double delete **/
        delete((this)->datamanager.get());
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
    static FIFO* make_new_fifo( const std::size_t n_items, 
                                const std::size_t align, 
                                void * const data )
    {
        if( data != nullptr )
        {
            return( 
                new RingBuffer<T, Type::Heap, false>( data, 
                                                      n_items, 
                                                      align /** actually start pos, redesign **/)
            );
        }
        else
        {
            return( new RingBuffer< T, Type::Heap, false >(n_items, align ) );
        }
    }

    virtual void resize(const std::size_t size, const std::size_t align,
        volatile bool& exit_alloc)
    {
        if((this)->datamanager.is_resizeable())
        {
            (this)->datamanager.resize(
                new Buffer::Data<T, type>(size, align), exit_alloc);
        }
        /** else, not resizeable..just return **/
        return;
    }

};


/**
 * RingBufferBaseMonitor - encapsulates logic for a queue with
 * monitoring enabled.
 */
template <class T, Type::RingBufferType type>
class RingBufferBaseMonitor : public RingBufferBase<T, type>
{
public:
    RingBufferBaseMonitor(const std::size_t n, const std::size_t align)
        : RingBufferBase<T, type>(), term(false)
    {
        (this)->datamanager.set(new Buffer::Data<T, Type::Heap>(n, align));
        (this)->init();
        /** add monitor types immediately after construction **/
        // sample_master.registerSample( new MeanSampleType< T, type >() );
        // sample_master.registerSample( new ArrivalRateSampleType< T, type >()
        // );
        // sample_master.registerSample( new DepartureRateSampleType< T, type >
        // () );
        //(this)->monitor = new std::thread( Sample< T, type >::run,
        //                                   std::ref( *(this)      /** buffer
        //                                   **/ ),
        //                                   std::ref( (this)->term /** term
        //                                   bool **/ ),
        //                                   std::ref( (this)->sample_master )
        //                                   );
    }

    void monitor_off()
    {
        (this)->term = true;
    }

    virtual ~RingBufferBaseMonitor()
    {
        (this)->term = true;
        // monitor->join();
        // delete( monitor );
        // monitor = nullptr;
        delete((this)->datamanager.get());
    }

    std::ostream& printQueueData(std::ostream& stream)
    {
        // stream << sample_master.printAllData( '\n' );
        return (stream);
    }

    virtual void resize(const std::size_t size, const std::size_t align,
        volatile bool& exit_alloc)
    {
        if((this)->datamanager.is_resizeable())
        {
            (this)->datamanager.resize(
                new Buffer::Data<T, type>(size, align), exit_alloc);
        }
        /** else, not resizeable..just return **/
        return;
    }

protected:
    // std::thread       *monitor;
    volatile bool term;
    // Sample< T, type >  sample_master;
};

template <class T>
class RingBuffer<T, Type::Heap, true /* monitor */>
    : public RingBufferBaseMonitor<T, Type::Heap>
{
public:
    /**
     * RingBuffer - default constructor, initializes basic
     * data structures.
     */
    RingBuffer(const std::size_t n, const std::size_t align = 16)
        : RingBufferBaseMonitor<T, Type::Heap>(n, align)
    {
        /** nothing really to do **/
    }

    virtual ~RingBuffer() = default;

    static FIFO* make_new_fifo( const std::size_t n_items, 
                                const std::size_t align, 
                                void * const data )
    {
        UNUSED( data );
        assert(data == nullptr);
        return( new RingBuffer<T, Type::Heap, true>(n_items, align) );
    }
};

/** specialization for dummy one **/
template <class T>
class RingBuffer<T, Type::Infinite, true /* monitor */>
    : public RingBufferBaseMonitor<T, Type::Infinite>
{
public:
    /**
     * RingBuffer - default constructor, initializes basic
     * data structures.
     */
    RingBuffer( const std::size_t n, 
                const std::size_t align = 16 )
        : RingBufferBaseMonitor<T, Type::Infinite>(1, align)
    {
        UNUSED( n );
    }

    virtual ~RingBuffer() = default;

    static FIFO* make_new_fifo( const std::size_t n_items, 
                                const std::size_t align, 
                                void const * data )
    {
        UNUSED( data );
        assert(data == nullptr);
        return (new RingBuffer<T, Type::Infinite, true>(n_items, align));
    }
};

/** specialization for dummy with no instrumentation **/
template <class T>
class RingBuffer< T, Type::Infinite, false >
    : public RingBufferBase< T, Type::Infinite >
{
public:
    /**
     * RingBuffer - default constructor, initializes basic
     * data structures.
     */
    RingBuffer( const std::size_t n, 
                const std::size_t align = 16)
        : RingBufferBase<T, Type::Infinite>()
    {
        UNUSED( n );
        (this)->datamanager.set(new Buffer::Data<T, Type::Heap>( 1, align ));
    }

    virtual ~RingBuffer()
    {
        delete((this)->datamanager.get());
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
    static FIFO* make_new_fifo( const std::size_t n_items, 
                                const std::size_t align, 
                                void * const data )
    {
        assert(data == nullptr);
        UNUSED( data );
        return (new RingBuffer<T, Type::Infinite, false>(n_items, align));
    }

    virtual void resize(
        const std::size_t size, 
        const std::size_t align, 
        bool& exit_alloc)
    {
        UNUSED( size );
        UNUSED( align );
        UNUSED( exit_alloc );

        /** should never really be calling this function **/
        assert(false);
    }

};



/**
 * SharedMemory
 */
template <class T>
class RingBuffer< T, Type::SharedMemory, false >
    : public RingBufferBase< T, Type::SharedMemory >
{
public:
    RingBuffer( const std::size_t nitems, 
                const std::string key, 
                const Direction   dir,
                const std::size_t alignment = 16 ) 
                    : RingBufferBase< T, 
                                      Type::SharedMemory >() ,
                      shm_key( key )
    {
        //TODO this needs to be an shm::open
        //TODO once open, needs to be an in-place allocate for
        //the first object to get the open, otherwise..use cookie
        //technique...all sub-keys will follow main key
        (this)->datamanager.set( 
            new Buffer::Data< T, Type::SharedMemory >( nitems, 
                                                       key, 
                                                       dir, 
                                                       alignment )
        );
    }

    virtual ~RingBuffer()
    {
        delete((this)->datamanager.get());
    }

    struct Data
    {
        const std::string key;
        Direction dir;
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
    static FIFO* make_new_fifo( const std::size_t n_items, 
                                const std::size_t align, 
                                void * const data )
    {
        auto * const data_ptr(reinterpret_cast<Data*>(data) );
        return( new RingBuffer< T, 
                                Type::SharedMemory, 
                                false>( n_items, 
                                        data_ptr->key, 
                                        data_ptr->dir, 
                                        align )
        );
    }

    virtual void resize( const std::size_t size, 
                         const std::size_t align,
                         volatile bool& exit_alloc )
    {
        UNUSED( size );
        UNUSED( align );
        UNUSED( exit_alloc );
        assert(false);
        /** TODO, implement me **/
        return;
    }


protected:
    const std::string shm_key;
};


#if 0
/**
 * TCP w/ multiplexing
 */
template <class T>
class RingBuffer<T, Type::TCP, false /* no monitoring yet */>
    : public RingBufferBase<T, Type::Heap>
{
public:
    RingBuffer( const std::size_t   nitems, 
                const std::string   dns_name,
                const Direction     dir, 
                const std::size_t   alignment = 16 ) 
                    : RingBufferBase<T, Type::Heap>()
    {
        // TODO, fill in stuff here, perhaps from original....
    }

    virtual ~RingBuffer() = default;

    struct Data
    {
        Direction dir;
        std::string dns_name;
    };

    static FIFO* make_new_fifo( const std::size_t n, 
                                const std::size_t align, 
                                void * const data )
    {
        auto * const  cast_data( reinterpret_cast< RingBuffer< T, 
                                                               Type::TCP, 
                                                               false >::Data* >(data) );

        return( new RingBuffer< T, 
                                Type::TCP, 
                                false >( n /** n_items **/, 
                                         cast_data->dns_name, 
                                         cast_data->dir, 
                                         align ) );
    }

    virtual void resize(    const std::size_t size, 
                            const std::size_t align,
                            volatile bool& exit_alloc)
    {
        UNUSED( size );
        UNUSED( align );
        UNUSED( exit_alloc );
        assert(false);
        /** TODO implement me **/
        return;
    }


protected:
};
#endif //TCP 

#endif /* END RAFTRINGBUFFER_TCC */
