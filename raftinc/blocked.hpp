/**
 * blocked.hpp -
 * @author: Jonathan Beard
 * @version: Sun Jun 29 14:06:10 2014
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
#ifndef RAFTBLOCKED_HPP
#define RAFTBLOCKED_HPP  1
#include <cstdint>
#include <cassert>
#include "defs.hpp"
#include "internaldefs.hpp"


struct ALIGN( L1D_CACHE_LINE_SIZE ) Blocked
{
    /** 
     * this is really the backign store for the number of 
     * times any queue in the system is blocked. Likely
     * this is plenty of storage given the purpose and 
     * no deleterious impact if we just wrap....
     */
    using value_type = std::uint32_t;
    using whole_type = std::uint64_t;
    
    static_assert( sizeof( value_type ) * 2 == sizeof( whole_type ),
                   "Error, the whole type must be double the size of the half type" );
    
    Blocked();

    Blocked( const Blocked &other );

    inline Blocked& operator = ( const Blocked &other ) noexcept
    {
        /** again, simple integer, let's ignore the case of other == this **/
        (this)->all = other.all;
        return( *this );
    }

    inline Blocked& operator += ( const Blocked &rhs ) noexcept
    {
        if( ! rhs.bec.blocked )
        {
            (this)->bec.count += rhs.bec.count;
        }
        return( (*this) );
    }
    
    struct blocked_and_counter
    {
       value_type   blocked = 0;
       value_type   count   = 0;
    };
    
    union
    {
        blocked_and_counter bec;
        whole_type          all = 0;
    };

    char pad[ L1D_CACHE_LINE_SIZE - sizeof( whole_type ) ]; 
}

;

#endif /* END RAFTBLOCKED_HPP */
