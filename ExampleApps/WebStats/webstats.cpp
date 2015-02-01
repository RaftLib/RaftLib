#include <iostream>
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
         const auto buff_length( 1000 );
         char buffer[ buff_length ];
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
            port.push( data_line );
         }
         return( raft::proceed );
      }
      else
      {
         return( raft::stop );
      }
   } );
   
   struct firstlastvisit
   {
      
   };
   /** q1 **/
   auto time_between = kernel::make< lambdak< webline > >( 1, 0, []( Port &input,
                                                                     Port &output )
   {
       
      return( raft::proceed );
   } );
   map.link( parser,
             kernel::make< lambdak< webline  > >( 1, 0, []( Port &input, 
                                                            Port &output )
             {
               auto in( input[ "0" ].pop_s< webline >() );
               std::cout << (*in).caltime.tm_year << " - " <<
                  (*in).caltime.tm_mon << " - " << 
                  (*in).caltime.tm_mday << " - " <<
                  (*in).caltime.tm_hour << ":" << 
                  (*in).caltime.tm_min  << ":" <<
                  (*in).caltime.tm_sec  << " - " << 
                  (*in).user << " - " << 
                  (*in).cat << "\n";
               return( raft::proceed );
             }) );

   map.exe();
   /** close stream **/
   stream.close();
   return( EXIT_SUCCESS );
}
