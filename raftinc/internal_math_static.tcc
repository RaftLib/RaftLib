/**
 * internal_math_static.tcc - 
 * @author: Jonathan Beard
 * @version: Wed Sep 16 08:26:15 2020
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
#ifndef INTERNAL_MATH_STATIC_TCC
#define INTERNAL_MATH_STATIC_TCC  1

#include <cstddef>
#include <type_traits>
#include <typeinfo>
#include <cstdint>

namespace raft
{

struct math
{
    constexpr static size_t log2( const std::size_t n ) noexcept 
    {
       return ( ( n<2 ) ? 1 : 1 + raft::math::log2( n / 2 ) );
    }

    constexpr static size_t get_bits_to_represent( const std::size_t val ) noexcept
    {
        return( raft::math::log2( val ) + 1 );
    }

};


//default
//template < class T > struct integer_width
//{   
//    using type_t = std::uint64_t;
//}

//8bit
//template < 


}

#endif /* END _INTERNAL_MATH_STATIC_TCC_ */
