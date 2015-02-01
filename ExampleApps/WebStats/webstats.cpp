#include <iostream>
#include <fstream>
#include <ctime>
#include <cstdint>
#include <raft>
#include <string>
#include <cinttypes>


int
main( int argc, char **argv )
{
   using namespace raft;
   struct webline
   {
      std::tm       caltime;
      std::uint64_t user;
      std::uint64_t cat;
   };
   std::ifstream stream( argv[ 1 ] );
   if( ! stream.is_open() )
   {
      std::cerr << "Couldn't open file: " << argv[ 1 ] << ", exiting!!\n";
      exit( EXIT_FAILURE );
   }
   /** parser kernel **/
   auto parser = kernel::make< lambdak< webline > >( 0, 1, [&]( Port &input, 
                                                                Port &output )
   {
      if( stream.good() )
      {
         webline     data_line;
         const auto  buff_length( 1000 );
         char        buffer[ buff_length ];
         stream.getline( buffer, buff_length );
         sscanf( buffer, "%d-%d-%d %d:%d:%d,%" PRIu64 ",%" PRIu64 "", 
                                                     &data_line.caltime.tm_year,
                                                     &data_line.caltime.tm_mon,
                                                     &data_line.caltime.tm_mday,
                                                     &data_line.caltime.tm_hour,
                                                     &data_line.caltime.tm_min,
                                                     &data_line.caltime.tm_sec,
                                                     &data_line.user,
                                                     &data_line.cat );

         for( auto &port : output )
         {
            if( stream.eof() )
            {
               port.push( data_line, raft::eof );
            }
            else
            {
               port.push( data_line );
            }
         }
         return( raft::proceed );
      }
      else
      {
         return( raft::stop );
      }
   } );
   

   /** q1 **/
   auto time_between = kernel::make< lambdak< webline > >( 1, 0, []( Port &input,
                                                                     Port &output )
   {
      static std::map< std::uint64_t, 
                       std::pair< std::time_t, std::time_t > > times;

      static std::uint64_t total( 0 );
      auto item( input[ "0" ].pop_s< webline >() );
      auto iterator( times.find( (*item).user ) );
      if( iterator == times.end() )
      {
         times.insert( std::make_pair( (*item).user,
                        std::make_pair( std::mktime( &(*item).caltime ), 0 ) ) ); 
                                        
      }
      else
      {
         auto &prev( (*iterator).second.second );
         const auto start( (*iterator).second.first );
         total -= prev;
         prev = std::difftime( std::mktime( &(*item).caltime ), start );
         total += prev;
      }
      if( item.sig() == raft::eof )
      {
         std::ofstream ofs( "Q1.csv", std::ofstream::out );
         if( ! ofs.is_open() )
         {
            std::cerr << "Failed to open output file for q1!\n";
         }
         ofs << (double) total / (double) times.size() << "\n";
         ofs.close();
         return( raft::stop );
      }
      return( raft::proceed );
   } );

   /** q2 **/

   map.link( parser, time_between );
   
   map.exe();
   /** close stream **/
   stream.close();
   return( EXIT_SUCCESS );
}
