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
   auto parser = kernel::make< lambdak< webline > >( 0, 2, [&]( Port &input, 
                                                                Port &output )
   {
      while( stream.good() )
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
            port.push( data_line, (stream.eof() ? raft::eof : raft::none ) );
         }
      }
      return( raft::stop );
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
   auto between_pages = kernel::make< lambdak< webline > >( 1, 0, []( Port &input,
                                                                      Port &output )
   {
      struct data
      {
         data( const std::time_t time ) : prev( time ){ };
         std::time_t   prev  = 0;
         std::time_t   total = 0;
         std::uint64_t N     = 0;

         data& operator << ( const std::time_t t )
         {
            const auto diff( std::difftime( t, prev ) );
            prev = t;
            total += diff;
            N++;
            return( (*this) );
         }
      };
      static std::map< std::uint64_t, data > times;

      auto item( input[ "0" ].pop_s< webline >() );
      auto iterator( times.find( (*item).user ) );
      if( iterator == times.end() )
      {
         times.insert( std::make_pair( (*item).user,
                       data(  std::mktime( &(*item).caltime ) ) )); 
                                        
      }
      else
      {
         (*iterator).second << std::mktime( &(*item).caltime );
      }
      if( item.sig() == raft::eof )
      {
         std::ofstream ofs( "Q2.csv", std::ofstream::out );
         if( ! ofs.is_open() )
         {
            std::cerr << "Failed to open output file for q1!\n";
         }
         double total( 0.0 );
         for( auto &pair : times )
         {
            if( pair.second.N not_eq 0 )
            {
               total += ( (double) pair.second.total / (double) pair.second.N );
            }
         }
         total /= times.size();
         ofs << total << "\n"; 
         ofs.close();
         return( raft::stop );
      }
      return( raft::proceed );
   } );

   map.link( parser, "0", time_between );
   map.link( parser, "1", between_pages );
   map.exe();
   /** close stream **/
   stream.close();
   return( EXIT_SUCCESS );
}
