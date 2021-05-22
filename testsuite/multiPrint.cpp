/**
 * random.cpp - 
 * @author: Jonathan Beard
 * @version: Mon Mar  2 14:00:14 2015
 * 
 * Copyright 2015 Jonathan Beard
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

#include <raft>
#include <raftrandom>
#include <cstdint>
#include <iostream>
#include <raftio>
#include "defs.hpp"
#include <sstream>
#include <iterator>

#define STATICPORT 4

template < class T > class sub : public raft::kernel
{
public:
    sub() : raft::kernel() 
    {
        input.addPort< T >( "0" );
        output.addPort< T >( "0" );
    }

    sub( const sub< T > &other ) : sub()
    {
        UNUSED( other );
    };

    virtual ~sub() = default;

    CLONE();

    virtual raft::kstatus run()
    {
        T a;
        input[ "0" ].pop( a );
        output[ "0" ].push( a - 10);
        return( raft::proceed );
    }
};


int
main()
{
    using namespace raft;
    using type_t = std::uint32_t;
    using gen = random_variate< std::default_random_engine,
                                std::uniform_int_distribution,
                                type_t >;
    using p_out = raft::print< type_t, ' ' >;
    
       
    const static auto min( 0 );
    const static auto max( 100 );
    gen g( max /** count to generate **/, min, max );
   
    std::stringstream ss;

    p_out print( ss, STATICPORT );
    

    raft::map m;
    m += g >= print;
    
    m.exe();

    const auto integer_count( 
        std::distance(
            std::istream_iterator<std::string>(ss), 
            std::istream_iterator<std::string>()
        )
     );
    std::cout << "counted \"" << integer_count << "\" integers\n";
    if( integer_count == STATICPORT * max )
    {
        return( EXIT_SUCCESS );
    }
    return( EXIT_FAILURE );
}
