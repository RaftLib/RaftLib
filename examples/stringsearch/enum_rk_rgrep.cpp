#include <map>
#define CORE_ASSIGN
std::map< std::uintptr_t, int > *core_assign = nullptr;
#include <iostream>
#include <cstdlib>
#include <string>
#include <algorithm>

#include <raft>
#include <raftio>
#include <raftutility>

#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <vector>
#include <iterator>
#include <fstream>
#include <random>

#include "searchdefs.hpp"
#include "search.tcc"
#include <unistd.h>


int
main( int argc, char **argv )
{
   core_assign = new std::map< std::uintptr_t, int >();
   std::default_random_engine generator;
   const auto maxcore(  sysconf(_SC_NPROCESSORS_ONLN) );
   std::uniform_int_distribution< int > distribution( 0 , maxcore );
   auto getcore( std::bind( distribution, generator ) );

   const auto num_threads( maxcore - 4 );
   
   if( argc != 4 )
   {  
      std::cerr << "usage: ./rgrep <SEARCH TERM> <TEXT FILE>\n";
      exit( EXIT_SUCCESS );
   }
   
   const std::string search_term( argv[ 1 ] );
   const std::string file( argv[ 2 ] );
  


   int fd( open( file.c_str(), O_RDONLY ) );
   if( fd < 0 )
   {
      perror( "Failed to open input file, exiting!!\n" );
      exit( EXIT_FAILURE );
   }
   struct stat st;
   if( fstat( fd, &st ) != 0 )
   {
      perror( "Failed ot open input file, exiting!!\n" );
      exit( EXIT_FAILURE );
   }

   char *buffer = nullptr;

   buffer = (char*) mmap( buffer,
         st.st_size,
         PROT_READ,
         ( MAP_PRIVATE ),
         fd,
         0 );
   if( buffer == MAP_FAILED )
   {
      perror( "Failed to mmap input file\n" );
      exit( EXIT_FAILURE );
   }
   close( fd );

   int ret_val( 0 );
   if( (ret_val = posix_madvise( buffer, 
                      st.st_size,
                      MADV_WILLNEED ) ) != 0 )
   {
      std::cerr << "return value: " << ret_val << "\n";
      perror( "Failed to give memory advise\n" );
      exit( EXIT_FAILURE );
   }

   
   /** set up complete enumeration of cores **/
   auto coreset( raft::range( 0, num_threads + 2, 1 ) );
   
   std::vector< raft::hit_t > total_hits;

   //auto kern_start( 
   //raft::map.link( 
   //   raft::kernel::make< raft::filereader< raft::chunk_t > >( file, 
   //                                                      num_threads,
   //                                                      search_term.length() ),
   //   raft::kernel::make< raft::search< raft::rabinkarp > >( search_term ) ) );

   auto *foreach( 
      raft::kernel::make< raft::for_each< char > >( buffer, st.st_size, num_threads ) );
   
   std::vector< raft::kernel* > rbk( num_threads );
   
   
   for( auto index( 0 ); index < num_threads; index++ )
   {
      rbk[ index ] = raft::kernel::make< 
         raft::search< raft::rabinkarp > >( search_term );
      raft::map.link( foreach, std::to_string( index ), rbk[ index ] );
   }
   
   std::vector< raft::kernel* > rbkverify( num_cores );
   for( index = 0; index < num_threads; index++ ) 
   {
      rbkverify[ index ] = 
         raft::kernel::make< raft::rkverifymatch >( buffer, st.st_size, search_term );
      raft::map.link( rbk[ index ], rbkverify[ index ] );
   }
   


   auto *filefinish(
      raft::kernel::make< raft::write_each< raft::hit_t > >(
         std::back_inserter( total_hits ), num_threads ) );

   for( auto index( 0 ); index < num_threads; index++ )
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
   std::cout << "Hits: " << total_hits.size() << "\n";
   for( raft::hit_t &val : total_hits )
   {
      std::cout << val << "\n"; 
      //": " << val.seg << "\n";
   }
   munmap( buffer, st.st_size );
   delete( core_assign );
   return( EXIT_SUCCESS );
}
