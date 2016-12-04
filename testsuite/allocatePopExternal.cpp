/**
 * allocateSendPush.cpp - throw an error if internal object
 * pop fails.
 *
 * @author: Jonathan Beard
 * @version: Sat Feb 27 19:10:26 2016
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
#include <cstdint>
#include <cstdio>
#include <cstddef>
#include <raft>
#include <cstdlib>
#include <cassert>

static std::vector< std::uintptr_t > A;
static std::vector< std::uintptr_t > B;
static std::vector< std::uintptr_t > C;

template < std::size_t N > struct foo
{
   foo() : length( N ){}

   foo( const foo &other ) : length( other.length )
   {
        using index_type = std::remove_const_t<decltype(N)>;
        for( index_type i( 0 ); i < N; i++ )
        {
            pad[ i ] = other.pad[ i ];
        }
   }

   ~foo() = default;

   int length;
   int pad[ N ];
};

using obj_t = foo< 80 >;

class start : public raft::kernel
{
public:
    start() : raft::kernel()
    {
        output.addPort< obj_t >( "y" );
    }

    virtual ~start() = default;

    virtual raft::kstatus run()
    {
        auto &mem( output[ "y" ].allocate< obj_t >() );
        A.emplace_back( reinterpret_cast< std::uintptr_t >( &mem ) ); 
        for( auto i( 0 ); i < mem.length; i++ )
        {
            mem.pad[ i ] = static_cast< int >( counter );
        }
        output[ "y" ].send();
        counter++;
        if( counter == 200 )
        {
            return( raft::stop );
        }
        return( raft::proceed );
    }

private:
    std::size_t counter = 0;
};


class middle : public raft::kernel
{
public:
    middle() : raft::kernel()
    {
        input.addPort< obj_t >( "x" );
        output.addPort< obj_t >( "y" );
    }

    virtual raft::kstatus run()
    {
        auto &val( input[ "x" ].peek< obj_t >() );
        B.emplace_back( reinterpret_cast< std::uintptr_t >( &val ) ); 
        output[ "y" ].push( val );
        input[ "x" ].unpeek();
        input[ "x" ].recycle( 1 );
        return( raft::proceed );
    }
};


class last : public raft::kernel
{
public:
    last() : raft::kernel()
    {
        input.addPort< obj_t >( "x" );
    }

    virtual ~last() = default;

    virtual raft::kstatus run()
    {
        obj_t mem;
        input[ "x" ].pop( mem );
        C.emplace_back( reinterpret_cast< std::uintptr_t >( &mem ) ); 
        /** Jan 2016 - otherwise end up with a signed/unsigned compare w/auto **/
        using index_type = std::remove_const_t<decltype(mem.length)>;
        for( index_type i( 0 ); i < mem.length; i++ )
        {
            //will fail if we've messed something up
            if( static_cast<std::size_t>(mem.pad[ i ]) != counter )
            {
                std::cerr << "failed test\n"; 
                exit( EXIT_FAILURE );
            }
        }
        counter++;
        return( raft::proceed );
    }

private:
    std::size_t counter = 0;
};

int
main()
{
    start s;
    last l;
    middle m;

    raft::map M;
    M += s >> m >> l;
    M.exe();
    for( std::size_t  i( 0 ); i < A.size(); i++ )
    {
        if( ( A[ i ] != B[ i ] ) && ( B[ i ] == C[ i ] ) )
        {
            std::cout << "test failed, first, middle should be equal, last should be diff & always the same\n";
            std::cout << std::hex << A[ i ] << " - " << B[ i ] << " - " << C[ i ] << "\n";
            return( EXIT_FAILURE );
        }
    }
    return( EXIT_SUCCESS );
}
