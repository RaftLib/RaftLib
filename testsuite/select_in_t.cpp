/**
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
#include <cstdint>
#include <iostream>
#include <raftio>
#include "generate.tcc"

using type_t = std::int32_t;

class dummy : public raft::kernel
{
public:
    dummy() : raft::kernel()
    {
        input.addPort< type_t >( "x_1", "x_2", "x_3" );
        output.addPort< type_t >( "y_1" );
    }

    /**
     * simple test case to select_in
     */
    virtual raft::kstatus run() override 
    {
        auto ret_val = raft::select::in( input, "x_1", "x_2", "x_3" );
        if( ret_val.first > 0 )
        {
            type_t x;
            ret_val.second.get().pop( x );
            output[ "y_1" ].push( x );
        }
        return( raft::proceed );
    }
};

int
main()
{
    using gen   = raft::test::generate< type_t >;
    using print = raft::print< type_t  , '\n' >;
    
    gen   a1;
    gen   a2;
    gen   a3;

    dummy d;  

    print p;
    raft::map m;

    m += a1 >> d[ "x_1" ];
    m += a2 >> d[ "x_2" ];
    m += a3 >> d[ "x_3" ];

    m += d >> p;

    m.exe();
    return( EXIT_SUCCESS );
}
