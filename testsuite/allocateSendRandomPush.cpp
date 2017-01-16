/**
 * allocateSendPush.cpp -
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

   int  length;
   int  pad[ N ];
};

using obj_t = foo< 100 >;

class start : public raft::kernel
{
public:
    start() : raft::kernel()
    {
        output.addPort< obj_t >( "y" );
    }

    virtual ~start() = default;

    IMPL_CLONE();
    virtual raft::kstatus run()
    {
        auto &mem( output[ "y" ].allocate< obj_t >() );
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

    virtual ~middle() = default;

    IMPL_CLONE();
    virtual raft::kstatus run()
    {
        auto &mem( input[ "x" ].peek< obj_t >() );
        if( send )
        {
            output[ "y" ].push( mem );
            send = false;
        }
        else
        {
            send = true;
        }
        input[ "x" ].unpeek();
        input[ "x" ].recycle();
        return( raft::proceed );
    }
private:
    bool send = true;
};


class last : public raft::kernel
{
public:
    last() : raft::kernel()
    {
        input.addPort< obj_t >( "x" );
    }

    virtual ~last() = default;

    IMPL_CLONE();
    virtual raft::kstatus run()
    {
        auto &mem( input[ "x" ].peek< obj_t >() );
        for( auto i( 0 ); i < mem.length; i++ )
        {
            //will fail if we've messed something up
            assert( static_cast<std::size_t>(mem.pad[ i ]) == counter );
        }
        input[ "x" ].unpeek();
        input[ "x" ].recycle();
        counter += 2;
        return( raft::proceed );
    }

private:
    std::size_t counter = 0;
};

int
main()
{
    start s;
    middle m;
    last l;

    raft::map M;
    M += s >> m >> l;
    M.exe();
    return( EXIT_SUCCESS );
}
