#include <iostream>
#include <cstdlib>
#include <string>

#include <raft>
#include <raftio>

#include <vector>
#include <iterator>

#include "search.tcc"

int
main( int argc, char **argv )
{
   const std::string search_term( argv[ 1 ] );
   const std::string file( argv[ 2 ] );

   const std::size_t num_threads( 1 );

   std::vector< raft::hit_t > total_hits; 
   
   auto kern_start( 
   raft::map.link( 
      raft::kernel::make< raft::filereader< raft::chunk_t > >( file, 
                                                         num_threads,
                                                         search_term.length() ),
      raft::kernel::make< raft::search< raft::rabinkarp > >( search_term ) ) );

   raft::map.link(
      &(kern_start.dst),
      raft::kernel::make< raft::write_each< raft::hit_t > >( std::back_inserter( total_hits ) ) );

   raft::map.exe();
   for( auto val : total_hits )
   {
      std::cerr << val << "\n";
   }
   return( EXIT_SUCCESS );
}
