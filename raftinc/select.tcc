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

namespace raft
{



/**
 * select_t - type to return from select, the first
 * index is the count of items or capcity to write into
 * (depending on the direction of the call returning this
 * pair), the second is a pointer to a FIFO object.
 */
using select_t = std::pair< std::size_t, FIFO* >;

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

/**
 * convert_arr - used to convert the forwared parameter pack 
 * to something that's non const 
 */
template < class... F > 
static auto convert_arr( F&&... t )
    -> std::array< 
        std::reference_wrapper< FIFO >, 
        sizeof...(F)
        >
{
    return { std::forward< F >( t )... };
}

/**
 * select_in - returns a random port that has data from the ports
 * given as parameters, will return a std::pair that has both the
 * count of items available on the input port and then the second
 * element is the port itself as a FIFO&. Uses mt19937 to pick a 
 * uniform random port. We'll add better policies in future versions. 
 * @param ports - parameter pack of ports
 * @return raft::select_t object, std::pair< count of items, FIFO&>
 */
template < class... PORTS > raft::select_t select_in( PORTS&&... ports )
{
    auto port_array 
        = convert_arr( std::forward< PORTS >( ports )... );
    std::random_device rd;
    std::mt19937 g( rd() );
    std::shuffle( port_array.begin(), 
                  port_array.end(), 
                  g );
    for( auto &p : port_array )
    {
        const auto _s = p.get().size();
        if( _s > 0 )
        {
            return( std::make_pair( _s, &(p.get()) ) );
        }
    }
    return( std::make_pair( 0, nullptr ) );
}

} /** end namespace raft **/
#endif /* END SELECT_TCC */
