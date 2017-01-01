/**
 * raftmanip.hpp - 
 * @author: Jonathan Beard
 * @version: Tue Aug 16 09:56:56 2016
 * 
 * Copyright 2016 Jonathan Beard
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
#ifndef _RAFTMANIP_HPP_
#define _RAFTMANIP_HPP_  1

#include <string>
#include <utility>
#include <cassert>
#include "defs.hpp"

namespace raft
{

class kernel;

/** BEGIN MANIP SETTERS FOR VALUE **/
template < manip_vec_t... VECLIST > struct manip_helper{};

/** catch tail end of recursion **/
template <> struct manip_helper<>
{
    constexpr static manip_vec_t get_value()
    {
        return( static_cast< manip_vec_t >( 0 ) );
    }
};

template < manip_vec_t N, manip_vec_t... VECLIST > struct manip_helper< N, VECLIST... >
{
    constexpr static manip_vec_t get_value( )
    {
        /** basic sanity check **/
        static_assert( N <= ( sizeof( manip_vec_t ) * raft::bits_per_byte ), 
            "Stream manipulator can only have 64 states [0:63], please check code" );
        return( 
            ( static_cast< manip_vec_t >( 1 ) << N ) | 
            manip_helper< VECLIST... >::get_value() ); 
    }
};
/** END MANIP SETTERS FOR VALUE **/
/** BEGIN HELPERS FOR BIND **/
template < manip_vec_t value, class... KERNELS > struct bind_helper{};

/** 
 * struct that doesn't do anything, just catch the end
 * condition for the recursion 
 */
template < manip_vec_t value > struct bind_helper< value >
{
    constexpr static void bind()
    {
        return;
    }
};

struct  manip_local
{
    static void apply_help( const manip_vec_t value, raft::kernel &k );
};

/** END HELPERS TO BIND **/
template < manip_vec_t value, class KERNEL, class... KERNELS > 
    struct bind_helper< value, KERNEL, KERNELS... >
{
    static void bind( KERNEL &&kernel, KERNELS&&... kernels )
    {
        /** recursively call for each kernel **/
        manip_local::apply_help( value, kernel );
        bind_helper< value, KERNELS... >::bind( 
            std::forward< KERNELS >( kernels )... );
        return;
    }
};


template < manip_vec_t... VECLIST > struct manip
{
    /**
     * value set at compile time so that all kernels that
     * have had a programmer override have the appropriate
     * vector stored. Can be set in-line to the stream, or
     * can be set via constructor.
     */
    constexpr static manip_vec_t value      
        = manip_helper< VECLIST... >::get_value();
    
    /**
     * bind - use this function to set the programmer specified
     * manipulate settings (stream manipulate) for the kernel,
     * future versions will return a ref wrapper object so that
     * the wrapped version can be caught with std::tie, but 
     * setting "decorator" style works for now and will be maintained
     * going forward.
     * @param kernels - parameter pack for kernels
     */
    template < class... KERNELS > static void bind( KERNELS&&... kernels )
    {
       
        bind_helper< value, KERNELS... >::bind( 
            std::forward< KERNELS >( kernels )... );
        return;
    }
};

} /** end namespace raft **/

#endif /* END _RAFTMANIP_HPP_ */
