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
#include "defs.hpp"
#include "internaldefs.hpp"

struct ALIGN( L1D_CACHE_LINE_SIZE ) Blocked
{
    using value_type = std::uint32_t;
    using whole_type = std::uint64_t;
    
    Blocked(){};

    Blocked( const Blocked &other ) : all( other.all ){}

    Blocked& operator += ( const Blocked &rhs ) noexcept;

    struct blocked_and_counter
    {
       value_type    blocked = 0;
       value_type    count   = 0;
    };
    
    union
    {
        blocked_and_counter bec;
        whole_type          all = 0;
    };

    raft::byte_t pad[ L1D_CACHE_LINE_SIZE - sizeof( whole_type ) ]; 
};

#endif /* END _BLOCKED_HPP_ */
