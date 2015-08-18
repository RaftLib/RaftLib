#include "portmap_t.hpp"
#include <pthread.h>

portmap_t::portmap_t()
{
   pthread_mutex_init( &mutex_map, nullptr );
}


portmap_t::~portmap_t()
{
   pthread_mutex_destroy( &mutex_map );
}
