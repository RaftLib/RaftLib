#include <thread>
#include "partition_basic.hpp"


void
partition_basic::partition( kernelkeeper &c )
{
    const auto num_cores( std::thread::hardware_concurrency() );
    core_id_t i( 0 );
    auto &container( c.acquire() );
    for( auto *kernel : container )
    {
       (this)->setCore( *kernel, i );
       //FIXME, too simple should be based on locality if hwloc avail
       i = ( i + 1 ) % num_cores;
    }
    c.release();
    return;
}
