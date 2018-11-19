#include <cassert>
#include <iostream>
#include <cstdint>
#include <cstdlib>
#include <raft>
#include <raftio>
#include <limits>
#include <unistd.h>


static bool global_bool = false;

struct SomeNonTrivialObject
{

    SomeNonTrivialObject()
    {
        value = nullptr;
    }

    SomeNonTrivialObject( std::int64_t *value )
    {
        if( value != nullptr )
        {
            (this)->value = value;
        }
    }


    ~SomeNonTrivialObject()
    {
        *value = std::numeric_limits< std::int64_t >::max();
    }

    SomeNonTrivialObject( const SomeNonTrivialObject &other )
    {
        /** let's pass the pointer to this object so we can see if it's deleted **/
        (this)->value = other.value;
    }


    std::int64_t *value = nullptr;
    
    /** padding to trigger external allocate **/
    char padding[ 
#ifdef PAGE_SIZE
    PAGE_SIZE << 1 
#elif defined (_SC_PAGESIZE )
    _SC_PAGESIZE << 1
#else
    1 << 30
#endif
    ];
};


/**
 * Idea is that we want to test if the destructor is called
 * on the object inside the port when popped on the remote
 * side. The consequence of not calling the destructor is that
 * memory if allocated outside the object could leak...which 
 * would be bad. 
 * 
 * what the test case does: we're passing in a pointer 
 * that is passed by value in the copy constructor, so
 * it should be the same from start to finish, pointing
 * to the int on the stack, however, if the destructor
 * is called, then the value should have '1' scribbled
 * on the memory that was the location.
 */

class source : public raft::kernel
{
public:
    source( SomeNonTrivialObject * obj ) : the_obj (obj)
    {
        output.addPort< SomeNonTrivialObject >( "out" );
    }

    virtual raft::kstatus run()
    {
         /** 
          * allocate passes the arguments to the constructor
          */
         output[ "out" ].allocate< SomeNonTrivialObject >( *the_obj  ); 
         output[ "out" ].send();
         return( raft::stop );
    }

    SomeNonTrivialObject    *the_obj;
};

class sink : public raft::kernel
{
public:
    sink()
    {
        input.addPort< SomeNonTrivialObject >( "in" );
    }

    virtual raft::kstatus run()
    {
        SomeNonTrivialObject test;
        input[ "in" ].pop( test );
        /** ok, let's see if we get all one's or something else **/
        if( (*test.value) == std::numeric_limits< std::int64_t >::max() )
        {
            global_bool = true;
        }
        else
        {
            global_bool = false;
        }
        return( raft::proceed );
    }
};


int
main( )
{
    std::int64_t the_value = 100;

    SomeNonTrivialObject sto( &the_value );
    
    raft::map m;
    source  s( &sto );
    sink    t;
    m += s >> t;
    m.exe();
    if( global_bool == true )
    {
        return( EXIT_SUCCESS );
    }
    else
    {
        return( EXIT_FAILURE );
    }
}
