/**
 * prefetchBench.cpp - 
 * @author: Jonathan Beard
 * @version: Fri Mar 25 14:49:51 2016
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

#include <raft>
#include <iostream>
#include <raftio>
#include <cstdlib>

#ifdef PAPITEST
#include <papi.h>
#include <papiStdEventDefs.h>

template < class FUNCTION,
           typename RETVAL,
           class FNAME,
           typename LINE,
           class... Args >
static void fun_function( FUNCTION function,
                          RETVAL val,
                          FNAME     filename,
                          LINE      linenumber,
                          Args&&... params )
{
    RETVAL ret( static_cast< RETVAL >( 0 ) );
    if( ( ret = function( std::forward< Args >( params )... ) ) != val )
    {
        std::fprintf( stderr, "Error %s %s '@' line %d:\n", PAPI_strerror( ret ),
                                                            filename,
                                                            linenumber );
        exit( ret );
    }
    return;
}
#endif



template < std::size_t N > struct foo
{
   constexpr foo() : length( N ){}
   
   foo( const foo &other ) : length( other.length )
   {
        for( auto i( 0 ); i < N; i++ )
        {
            pad[ i ] = other.pad[ i ];
        }
   }

   ~foo() = default;

   int  length;
   int  pad[ N ];
};


using obj_t = foo< 1 << 16 >;

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
        for( auto i( 0 ); i < mem.length; i++ )
        {
            mem.pad[ i ] = counter;
        }
        output[ "y" ].send();
        counter++;
        if( counter == 100000 )
        {
            return( raft::stop );
        }
        return( raft::proceed );
    }

private:
    std::size_t counter = 0;
};

class passthrough : public raft::kernel
{
public:
    passthrough() : raft::kernel()
    {
        input.addPort< obj_t >( "in" );
        output.addPort< obj_t >( "out" );
#ifdef PAPITEST        
        //set up PAPI
        fun_function( PAPI_library_init,
                      PAPI_VER_CURRENT,
                      __FILE__,
                      __LINE__,
                      PAPI_VER_CURRENT );
        fun_function( PAPI_create_eventset,
                      PAPI_OK,
                      __FILE__,
                      __LINE__,
                      &EventSet );
        //add L1D Cache
        fun_function( PAPI_add_event,
                      PAPI_OK,
                      __FILE__,
                      __LINE__,
                      EventSet,
                      PAPI_L1_DCM );
        //add L2 Cache
        fun_function( PAPI_add_event,
                      PAPI_OK,
                      __FILE__,
                      __LINE__,
                      EventSet,
                      PAPI_L2_DCM );
        //start counters
        fun_function( PAPI_start,
                      PAPI_OK,
                      __FILE__,
                      __LINE__,
                      EventSet );
#endif                      
    }

#ifdef PAPITEST
    virtual ~passthrough()
    {
        const static auto num_events( 2 );
        long long values[ num_events ] = { 0ll, 0ll };
        fun_function( PAPI_stop,
                      PAPI_OK,
                      __FILE__,
                      __LINE__,
                      EventSet,
                      values );
        for( auto( i ); i < num_events; i++ )
        {
            std::cout << values[ i ];
            if( i != num_events - 1 )
            {
                std::cout << ", ";
            }
        }
        std::cout << "\n";
    }
#else
    virtual ~passthrough() = default;
#endif                      

    virtual raft::kstatus run()
    {
        auto &st( input[ "in" ].peek< obj_t >() );
        for( auto i( 0 ); i < st.length; i++ )
        {
            st.pad[ i ]++;
        }
        output[ "out" ].push( st );
        input[ "in" ].unpeek();
        input[ "in" ].recycle();
        return( raft::proceed );
    }

#ifdef PAPITEST    
private:
    int EventSet = PAPI_NULL;
#endif
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
        input[ "x" ].recycle();
        return( raft::proceed );
    }

private:
    std::size_t counter = 0;
};



int 
main( int argc, char **argv )
{
    start st;
    passthrough pt;
    last l;

    raft::map m;
    m += st >> pt >> l; 
    m.exe();
    return( EXIT_SUCCESS );
}
