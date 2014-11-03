#include <iostream>
#include <cstdlib>
#include <string>

#include <raft>
#include <raftio>

#include <vector>
#include <iterator>

#include "rkverifymatch.hpp"
#include "searchdefs.hpp"
#include "search.tcc"
   
namespace raft
{
   Map map;
}

int
main( int argc, char **argv )
{
   const std::string search_term( argv[ 1 ] );
   const std::string file( argv[ 2 ] );

   const std::size_t num_threads( 1 );

   std::vector< raft::match_t > total_hits; 
   

   auto kern_start( 
   raft::map.link( 
      raft::kernel::make< raft::filereader< raft::chunk_t > >( file, 
                                                         num_threads,
                                                         search_term.length() ),
      raft::kernel::make< raft::search< raft::rabinkarp > >( search_term ) ) );

   auto kern_mid(
   raft::map.link(
      &(kern_start.dst),
      raft::kernel::make< raft::rkverifymatch >( file, search_term ) ) ); 
      

   raft::map.link( 
      &(kern_mid.dst),
      raft::kernel::make< 
         raft::write_each< raft::match_t > >( 
            std::back_inserter( total_hits ) ) );
   raft::map.exe();
   for( auto &val : total_hits )
   {
      std::cout << val.hit_pos << ": " << val.seg << "\n";
   }
   return( EXIT_SUCCESS );
}
