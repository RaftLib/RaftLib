#include <cassert>
#include <iostream>
#include <cstdint>
#include <cstdlib>
#include <raft>
#include <raftio>
#include "non_trivial_obj_def.tcc"

static bool global_bool = false;

/** with true  param b/c we want the padding **/
using non_trivial_obj_t = raft::test::non_trivial_object< true >;


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
    source( non_trivial_obj_t * obj ) : the_obj (obj)
    {
        output.addPort< non_trivial_obj_t >( "out" );
    }

    

    virtual raft::kstatus run()
    {
         /** 
          * allocate passes the arguments to the constructor
          */
         output[ "out" ].allocate< non_trivial_obj_t >( *the_obj  ); 
         output[ "out" ].send();
         return( raft::stop );
    }

    non_trivial_obj_t    *the_obj;
};

class sink : public raft::kernel
{
public:
    sink()
    {
        input.addPort< non_trivial_obj_t >( "in" );
    }


    virtual raft::kstatus run()
    {
        non_trivial_obj_t test;
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

    non_trivial_obj_t sto( &the_value );
    
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
