#include <stdio.h>

#include "kernel.hpp"

using namespace raft;

/** default **/
kernel::kernel(){};

/** existing memory **/
kernel::kernel( void * const ptr, 
                const std::size_t nbytes ) : 
   input(  this, ptr, nbytes ),
   output( this, ptr, nbytes )
{
}
