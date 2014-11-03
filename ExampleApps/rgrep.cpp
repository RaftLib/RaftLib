#include <iostream>
#include <cstdlib>
#include <string>

#include <raft>
#include <raftio>

#include <array>
#include <vector>
#include <iterator>
#include <fstream>

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
   if( argc != 3 )
   {  
      std::cerr << "usage: ./rgrep <SEARCH TERM> <TEXT FILE>\n";
      exit( EXIT_SUCCESS );
   }
   
   const std::string search_term( argv[ 1 ] );
   const std::string file( argv[ 2 ] );
   
   std::cout << "Searching for: " << search_term << "\n";
   std::cout << "In filename: " << file << "\n";

   const std::size_t num_threads( 2 );

   std::vector< raft::match_t > total_hits; 
   

   //auto kern_start( 
   //raft::map.link( 
   //   raft::kernel::make< raft::filereader< raft::chunk_t > >( file, 
   //                                                      num_threads,
   //                                                      search_term.length() ),
   //   raft::kernel::make< raft::search< raft::rabinkarp > >( search_term ) ) );


   auto *filereader( raft::kernel::make< raft::filereader< raft::chunk_t > >(
      file,
      num_threads,
      search_term.length() ) );
   std::array< raft::kernel*, num_threads > rbk;
   std::size_t index( 0 );
   for( index = 0; index < num_threads; index++ )
   {
      rbk[ index ] = raft::kernel::make< 
         raft::search< raft::rabinkarp > >( search_term );
      raft::map.link( filereader, std::to_string( index ), rbk[ index ] );
   }
   std::array< raft::kernel*, num_threads > rbkverify;
   for( index = 0; index < num_threads; index++ ) 
   {
      rbkverify[ index ] = 
         raft::kernel::make< raft::rkverifymatch >( file, search_term );
      raft::map.link( rbk[ index ], rbkverify[ index ] );
   }

   auto *filefinish(
      raft::kernel::make< raft::write_each< raft::match_t > >(
         std::back_inserter( total_hits ), num_threads ) );

   for( index = 0; index < num_threads; index++ )
   {
      raft::map.link( rbkverify[ index ], 
                      filefinish, 
                      std::to_string( index ) );
   }
   

   //auto kern_mid(
   //raft::map.link(
   //   &(kern_start.dst),
   //   raft::kernel::make< raft::rkverifymatch >( file, search_term ) ) ); 
   //   

   //raft::map.link( 
   //   &(kern_mid.dst),
   //   raft::kernel::make< 
   //      raft::write_each< raft::match_t > >( 
   //         std::back_inserter( total_hits ) ) );
   
   raft::map.exe();

   //std::ofstream ofs("/dev/null");
   //for( raft::match_t &val : total_hits )
   //{
   //   ofs << val.hit_pos << ": " << val.seg << "\n";
   //}
   return( EXIT_SUCCESS );
}
