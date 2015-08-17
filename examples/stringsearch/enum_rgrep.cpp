#include <map>
#define CORE_ASSIGN
std::map< std::uintptr_t, int > *core_assign = nullptr;

#include <iostream>
#include <cstdlib>
#include <string>
#include <random>

#include <raft>
#include <raftio>

#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <vector>
#include <iterator>
#include <fstream>
#include <iomanip>

#include "searchdefs.hpp"
#include "search.tcc"

#include "Clock.hpp"
extern Clock *system_clock;

int
main( int argc, char **argv )
{
   std::default_random_engine generator;
   const auto maxcore(  sysconf( _SC_NPROCESSORS_CONF ) );
   std::uniform_int_distribution< int > distribution( 0 , maxcore );
   auto getcore( std::bind( distribution, generator ) );

   const auto num_threads( 5 );
   
   if( argc != 3 )
   {  
      std::cerr << "usage: ./rgrep <SEARCH TERM> <TEXT FILE>\n";
      exit( EXIT_SUCCESS );
   }
   
   const std::string search_term( argv[ 1 ] );
   const std::string file( argv[ 2 ] );
  
   auto assigncore( [&]( raft::kernel *k ) -> void
   {
      (*core_assign)[ reinterpret_cast< std::uintptr_t >( k ) ] = 
         getcore();
      return;
   } );

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
   /** data file **/
   std::ofstream results( "resultslog.csv" );
   /** now we need to iterate over all combinations of buffer sizes **/
   std::vector< std::size_t > buff_size( num_threads + 2 );
   std::fill( buff_size.begin(), buff_size.end(), 4 );
   const int max_buff_size( 4096 );
   for( auto &curr_buff : buff_size )
   {
      for( auto curr_size( curr_buff ); curr_size < max_buff_size; curr_size++ )
      {
         auto assign_index( 0 );
         Map localmap;
         core_assign = new std::map< std::uintptr_t, int >();
         std::vector< raft::hit_t > total_hits; 
         auto *foreach( 
            raft::kernel::make< 
               raft::for_each< char > >( buffer, st.st_size, num_threads ) );
         assigncore( foreach ); 
         std::vector< raft::kernel* > rbk( num_threads );
         for( auto index( 0 ); index < num_threads; index++ )
         {
            rbk[ index ] = raft::kernel::make< 
               raft::search< raft::boyermoore> >( search_term );
            localmap.link( foreach, 
                           std::to_string( index ), 
                           rbk[ index ],
                           buff_size[ assign_index++ ] );
            assigncore( rbk[ index ] );
         }
         

         auto *filefinish(
            raft::kernel::make< raft::write_each< raft::hit_t > >(
               std::back_inserter( total_hits ), num_threads ) );
         assigncore( filefinish );
         for( auto index( 0 ); index < num_threads; index++ )
         {
            localmap.link( rbk[ index ], 
                           filefinish, 
                           std::to_string( index ),
                           buff_size[ assign_index++ ] );
         }
         
         const auto start( system_clock->getTime() );
         localmap.exe();
         const auto end( system_clock->getTime() );
         for( const auto &pair : (*core_assign) )
         {
            results << pair.second << ", ";
         }
         for( const auto s : buff_size )
         {
            results << s << ", ";
         }
         results << std::setprecision( 10 );
         results << ( end - start ) << "\n";
         std::ofstream ofs("/dev/null");
         std::cout << "Hits: " << total_hits.size() << "\n";
         for( raft::hit_t &val : total_hits )
         {
            ofs << val << "\n"; 
         }
         ofs.close();
         delete( core_assign );
      }
   }
   results.flush();
   results.close();
   munmap( buffer, st.st_size );
   return( EXIT_SUCCESS );
}
