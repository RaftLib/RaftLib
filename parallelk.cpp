#include <cstddef>
#include "parallelk.hpp"
#include "pthreadwrap.h"
using namespace raft;

parallel_k::parallel_k()
{

}

parallel_k::parallel_k( void * const ptr ,
                        const std::size_t nbytes ) : kernel( ptr, nbytes )
{
}


parallel_k::~parallel_k()
{

}


void 
parallel_k::lock_helper( Port &port )
{
   pthread_mutex_lock( &port.portmap.mutex_map ); 
}

void 
parallel_k::unlock_helper( Port &port )
{
   pthread_mutex_unlock( &port.portmap.mutex_map ); 
}
