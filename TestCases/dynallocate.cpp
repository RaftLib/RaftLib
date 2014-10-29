#include <cassert>
#include <iostream>
#include <cstdint>
#include <cstdlib>
#include <raft>
#include <vector>
#include <functional>
#include <iostream>
#include <raft>
#include <cstdlib>
#include <chrono>
#include <thread>

template < typename T > class Generate : public raft::kernel
{
public:
   Generate( std::int64_t count = 1000 ) : raft::kernel(),
                                          count( count )
   {
      output.addPort< T >( "number_stream" );
   }

   virtual raft::kstatus run()
   {
      if( count-- > 1 )
      {
         output[ "number_stream" ].push( count );
         return( raft::proceed );
      }
      output[ "number_stream" ].push( count, raft::eof );
      return( raft::stop );
   }

private:
   std::int64_t count;
};

template< typename T, char delim = '\0' > class Print : public raft::kernel
{
public:
   Print( ) : raft::kernel()
   {
      input.addPort< T >( "in" );
   }

   /** 
    * run - implemented to take a single 
    * input port, pop the itam and print it.
    * the output isn't yet synchronized so if
    * multiple things are printing to std::cout
    * then there might be issues, otherwise
    * this works well for debugging and basic 
    * output.
    * @return raft::kstatus
    */
   virtual raft::kstatus run()
   {
      std::chrono::milliseconds duration( 2 );
      std::this_thread::sleep_for( duration );
      T data;
      raft::signal  signal( raft::none );
      input[ "in" ].pop( data, &signal );
      if( delim != '\0' )
      {
         std::cout << data << delim;
      }
      else
      {
         std::cout << data;
      }
      if( signal == raft::eof )
      {
         return( raft::stop );
      }
      return( raft::proceed );
   }
};

int
main( int argc, char **argv )
{
   using namespace raft;
   map.link( new Generate< std::int64_t >(), new Print< std::int64_t, '\n' >() );
   map.exe();
   return( EXIT_SUCCESS );
}



