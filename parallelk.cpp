#include <cstddef>
#include <mutex>
#include "parallelk.hpp"

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
   while( not port.portmap.mutex_map.try_lock() )
   {
      std::this_thread::yield();
   }
   //lock acquired
   return;
}

void 
parallel_k::unlock_helper( Port &port )
{
   port.portmap.mutex_map.unlock(); 
}
