#include <cassert>
#include <iostream>
#include <cstdint>
#include <cstdlib>
#include <raft>
#include <raftio>
#include <raftrandom>



int
main( int argc, char **argv )
{
   using gen = raft::random_variate< std::int64_t, raft::sequential >;
   raft::map.link( 
      raft::kernel::make< gen >( 1, 10000 ), 
      raft::kernel::make< raft::print< std::int64_t, '\n' > >() );
   raft::map.exe();
   return( EXIT_SUCCESS );
}
