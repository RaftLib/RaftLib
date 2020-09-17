/**
 * internalmath.cpp - 
 * @author: Jonathan Beard
 * @version: Wed Sep 16 08:38:37 2020
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


#include <iostream>
#include <cstdlib>
#include <cstdio>

#include "internal_math_static.tcc"

template < size_t X, size_t Y> struct tester
{
    constexpr static bool check() noexcept
    {
        return( raft::math::log2( X ) != Y );
    }

};

int main()
{
    if( tester< 4, 2 >::check() )
    {
        return( EXIT_FAILURE );
    }
    
    if( tester< 5, 2 >::check() )
    {
        return( EXIT_FAILURE );
    }
    
    if( tester< 16, 4 >::check() )
    {
        return( EXIT_FAILURE );
    }

    if( raft::math::get_bits_to_represent( 16 ) != 5 )
    {
        return( EXIT_FAILURE );
    }
    
    if( raft::math::get_bits_to_represent( 17 ) != 6 )
    {
        return( EXIT_FAILURE );
    }



    return( EXIT_SUCCESS );
}
