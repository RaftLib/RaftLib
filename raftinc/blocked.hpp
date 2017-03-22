/**
 * blocked.hpp -
 * @author: Jonathan Beard
 * @version: Sun Jun 29 14:06:10 2014
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
#ifndef _BLOCKED_HPP_
#define _BLOCKED_HPP_  1
#include <cstdint>
#include <cassert>


/** 
 * FIXME move thos code to the defs.hpp file
 * useful for other things, no point in duplicating
 * elsewhere 
 */
#ifdef _MSC_VER
#    if (_MSC_VER >= 1800)
#        define __alignas_is_defined 1
#    endif
#    if (_MSC_VER >= 1900)
#        define __alignof_is_defined 1
#    endif
#else 
#    ifndef __APPLE__
#    include <cstdalign>   // __alignas/of_is_defined directly from the implementation
#    endif
#endif

/**
 * should be included, but just in case there are some 
 * compilers with only experimental C++11 support still
 * running around, check macro..turns out it's #ifdef out 
 * on GNU G++ so checking __cplusplus flag as indicated
 * by https://goo.gl/JD4Gng
 */
#if ( __alignas_is_defined == 1 ) || ( __cplusplus >= 201103L )
#    define ALIGN(X) alignas(X)
#else
#    pragma message("C++11 alignas unsupported :( Falling back to compiler attributes")
#    ifdef __GNUG__
#        define ALIGN(X) __attribute__ ((aligned(X)))
#    elif defined(_MSC_VER)
#        define ALIGN(X) __declspec(align(X))
#    else
#        error Unknown compiler, unknown alignment attribute!
#    endif
#endif

#if ( __alignas_is_defined == 1 ) || ( __cplusplus >= 201103L )
#    define ALIGNOF(X) alignof(x)
#else
#    pragma message("C++11 alignof unsupported :( Falling back to compiler attributes")
#    ifdef __GNUG__
#        define ALIGNOF(X) __alignof__ (X)
#    elif defined(_MSC_VER)
#        define ALIGNOF(X) __alignof(X)
#    else
#        error Unknown compiler, unknown alignment attribute!
#    endif
#endif


/**
 * FIXME...should probably align these to cache line then 
 * zero extend pad for producer/consumer.
 */
struct ALIGN(64) Blocked
{
    using value_type = std::uint32_t;
    using whole_type = std::uint64_t;
    
    static_assert( sizeof( value_type ) * 2 == sizeof( whole_type ),
                   "Error, the whole type must be double the size of the half type" );
    Blocked() = default;

    Blocked( const Blocked &other ) : all( other.all ){}

    Blocked& operator += ( const Blocked &rhs )
    {
       if( ! rhs.bec.blocked )
       {
          (this)->bec.count += rhs.bec.count;
       }
       return( *this );
    }
    struct blocked_and_counter
    {
       value_type   blocked;
       value_type    count;
    };
    
    union
    {
        blocked_and_counter bec;
        whole_type          all = 0;
    };

    char pad[ L1D_CACHE_LINE_SIZE - sizeof( whole_type ) ]; 
}

;

#endif /* END _BLOCKED_HPP_ */
