#include <stdio.h>
#include <cxxabi.h>

#include "kernel.hpp"

using namespace raft;

std::size_t kernel::kernel_count( 0 );

/** default **/
kernel::kernel() : kernel_id( kernel::kernel_count )
{
   kernel::kernel_count++;
};

/** existing memory **/
kernel::kernel( void * const ptr, 
                const std::size_t nbytes ) : 
   input(  this, ptr, nbytes ),
   output( this, ptr, nbytes ),
   kernel_id( kernel::kernel_count )
{
}

std::size_t
kernel::get_id()
{
   return( kernel_id );
}

std::int32_t
kernel::setRunningState( raft::kernel * const k )
{
   return( static_cast< 
            std::int32_t >( setjmp( k->running_state ) ) );
}

std::int32_t
kernel::setPreemptState( raft::kernel * const k )
{
   return( static_cast< 
            std::int32_t >( setjmp( k->preempt_state ) ) );
}

void
kernel::preempt( raft::kernel * const k )
{
   longjmp( k->preempt_state , 1 );
}

//std::string
//kernel::getName()
//{
//   int status( 0 );
//   const std::string name_a( 
//      abi::__cxa_demangle( typeid( *(this) ).name(), 0, 0, &status ) );
//
//}
