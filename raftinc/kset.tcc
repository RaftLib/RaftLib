/**
 * kset.tcc - class to contain a set of kernels, this one
 * only exists temporarily in order to make construction
 * of complex graphs relatively simple.
 *
 * @author: Jonathan Beard
 * @version: Wed Apr 13 12:50:38 2016
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
#ifndef _KSET_TCC_
#define _KSET_TCC_  1
#include <type_traits>
#include <typeinfo>
#include <iostream>
#include <cstdlib>
#include <vector>
#include <cassert>
#include <functional>
#include "common.hpp"


namespace raft
{

/** predeclare raft::kernel **/
class kernel;

/**
 * dummy class for extending to kset, largely used
 * to keep from having to type out the class type
 * in long form within the kpair/map classes.
 */
struct basekset{
    basekset()          = default;
    virtual ~basekset() = default;

    
    using container_type = 
        std::vector< std::reference_wrapper< raft::kernel > >;

    using iterator = container_type::iterator; 
    
    using const_iterator = container_type::const_iterator;

    //virtual iterator         begin()      = 0;
    virtual const_iterator   cbegin()     = 0;
    //virtual iterator         end()        = 0;
    virtual const_iterator   cend()       = 0;
};

/** pre-declaration **/
template < class... KERNELS > struct AddKernel;

/**
 * feel hacky doing this, but it's probably the
 * least amount of code I can write to get this
 * done....probably better way using a tuple
 * like construct, then the compiler would do
 * most of the work
 */

template < class... K > class ksetr
#ifndef TEST_WO_RAFT
: public basekset
#endif
{
private:
    static_assert( sizeof...( K ) > 0, "size for kset must be > 0" );
    using common_t = typename std::common_type< K... >::type;
#ifndef TEST_WO_RAFT
    static_assert( std::is_base_of< raft::kernel,
                                    common_t >::value,
                                    "All kernels given to kset must be derived from raft::kernel" );
#endif
    /** don't want to type this over and over **/
    using vector_t = std::vector< std::reference_wrapper< common_t > >;
    vector_t  k;

public:

#ifdef TEST_WO_RAFT
using const_iterator = typename vector_t::const_iterator;
using iterator       = typename vector_t::iterator;
#else
using const_iterator = typename basekset::const_iterator;
using iterator       = typename basekset::iterator;
#endif
    /**
     * base constructor, with multiple args.
     */
    ksetr( K&... kernels )
    {
        /**
         * until we get the tuple-like solution built, dynamic alloc
         * will work, just use reserve to keep from allocating for
         * every subsequent call
         */
        k.reserve( sizeof...( K ) );
        AddKernel< K... >::add( std::forward< K >( kernels )..., k );
    }

    /**
     * move constructor, hopefully keep the dynamic mem
     * in vector valid w/o re-allocating and copying.
     */
    ksetr( ksetr< K... > &&other ) : k( std::move( other.k ) ){}

    /**
     * a bit hacky, but wanted to use iterators, seems
     * like this is the siplest way since the compiler
     * will only check for begin and end.
     */
    virtual const_iterator cbegin()
    {
        return( k.cbegin() );
    }
    
    virtual iterator begin()
    {
        return( k.begin() );
    }

    /**
     * returns end iterator
     */
    virtual const_iterator  cend()
    {
        return( k.cend() );
    }
    
    virtual iterator  end()
    {
        return( k.end() );
    }


    /**
     * get the number of kernels held.
     */
    constexpr typename vector_t::size_type size() const { return( sizeof...( K ) ); }

    /**
     * get the common, base class shared between
     * the kernels, probably raft::kernel but maybe
     * not.
     */
    using value_type = common_t;
};


/**
 * resursive AddKernel, keep going
 */
template < class K, class... KS > struct AddKernel< K, KS... >
{
    template < class CONTAINER >
        static void add( K &&kernel,
                         KS&&... kernels,
                         CONTAINER &c )
    {
        c.emplace_back( kernel );
        AddKernel< KS... >::add( std::forward< KS >( kernels )..., c );
        return;
    }
};

/**
 * base case for recursive AddKernel, stop
 * the recursion.
 */
template < class K > struct AddKernel< K >
{
    template < class CONTAINER >
        static void add( K &&kernel, CONTAINER &c )
    {
        c.emplace_back( kernel );
        return;
    }
};

/**
 * wrap so I can get away w/o having to do the class vs. constructor
 * template args
 */
template< class... K >
inline
static
ksetr< K... >
kset( K&&... kernels )
{
    return( ksetr< K... >( std::forward< K >( kernels )... ) );
}

} /** end namespace raft */
#endif /* END _KSET_TCC_ */
