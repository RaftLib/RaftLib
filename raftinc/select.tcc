/*
 * select.tcc - 
 * @author: Jonathan Beard
 * @version: Thu Mar  4 07:00:04 2021
 * 
 * Copyright 2021 Jonathan Beard
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
#ifndef SELECT_TCC
#define SELECT_TCC  1

#include "fifo.hpp"
#include <utility>
#include <memory>
#include <functional>
#include <algorithm>
#include <random>
#include <array>
#include <string>
#include "common.hpp"

namespace raft
{



/**
 * select_t - type to return from select, the first
 * index is the count of items or capcity to write into
 * (depending on the direction of the call returning this
 * pair), the second is a pointer to a FIFO object.
 */
using select_t = std::pair< std::size_t, std::reference_wrapper< FIFO > >;

/**
 * TODO, finish this version, more fun and a bit more efficent
 * than the loop version. 
 */
#if 0
template < class... PORTS > struct select_helper{};

template < class PORT > struct select_helper < PORT >
{
    static select_t select_in( PORT &&port )
    {
        const auto _s = port.size();
        if( _s > 0 )
        {
            return( std::make_pair( _s, std::make_unique< FIFO >( &port ) ) );
        }
    }
};

template < class PORT, class... PORTS > struct select_helper< PORT, PORTS... >
{
    static PORT& select_in( PORT &&port, PORTS&&... ports )
    {
        if( port.size() > 0 )
        {
            return( 
        }
    }
};
#endif


struct select{

/**
 * select_in - returns a random port that has data from the ports
 * given as parameters, will return a std::pair that has both the
 * count of items available on the input port and then the second
 * element is the port itself as a FIFO&. Uses knuth_b to pick a 
 * random port in an attempt to provide some level of "fairness". 
 * We'll add better policies in future versions. 
 * @param port_container - the port that you're expecting data from
 * @param ports - parameter pack of ports as strings
 * @return raft::select_t object, std::pair< count of items, FIFO&>
 */
template < class PORT_CONTAINER, 
           class... PORT_NAMES > 
static 
raft::select_t in( PORT_CONTAINER  &&port_container, 
                          PORT_NAMES&&... ports )
{
    auto port_array 
        = common::convert_arr( std::forward< PORT_NAMES >( ports )... );
    /**
     * only need to initialize/create these once.
     */
    static std::random_device rd;
    static std::knuth_b g( rd() );
    /**
     * shuffle
     */
    std::shuffle( port_array.begin(), 
                  port_array.end(), 
                  g );

    for( auto &p : port_array )
    {
        auto &fifo      = port_container[ p ];
        const auto _s   = fifo.size();
        if( _s > 0 )
        {
            /** 
             * keep in mind the return is actually a 
             * reference wrapper, it's not really a 
             * raw pointer being returned. 
             */
            return( std::make_pair( _s, std::ref( fifo ) ) );
        }
    }
    /**
     * return any fifo, doesn't really matter given the 
     * semantics of the function say that you must check
     * the first index before accessing the FIFO itself.
     */
    auto &dummy_fifo = port_container[ port_array[ 0 ] ];
    return( std::make_pair( 0, std::ref( dummy_fifo ) ) );
}

};

} /** end namespace raft **/
#endif /* END SELECT_TCC */
