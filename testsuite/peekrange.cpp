#include <cassert>
#include <iostream>
#include <cstdint>
#include <cstdlib>
#include <vector>
#include <iterator>
#include <raft>

template< typename T > class print : public raft::kernel
{
public:
   print() : raft::kernel()
   {
      input.addPort< T >( "in" );
   }

   virtual raft::kstatus run()
   {
      auto &port( input[ "in" ] );
      try
      {
         auto range( port.template peek_range< T >( 5 ) );
         for( int i( 0 ); i < 5; i++ )
         {
            std::cout << " " << range[ i ].ele;
         }
      } 
      catch( NoMoreDataException &ex )
      {
         std::cerr << ex.what() << "\n";
         /** nothing bad, just no more data **/
         return( raft::stop );
      }
      catch( ClosedPortAccessException &ex )
      {
         /** nothing bad, just no more data **/
         return( raft::stop );
      }
      std::cout << "\n";
      port.recycle( 2 );
      return( raft::proceed );
   }
};

namespace raft
{
   Map map;
}

#define COUNT 20

int
main( int argc, char **argv )
{
   int *arr = (int*) malloc( sizeof( int ) * COUNT );
   for( int i( 0 ); i < COUNT; i++ )
   {
      arr[ i ] = i;
   }


   raft::map.link( 
      raft::kernel::make< raft::for_each< int > >( arr, COUNT, 1),
      raft::kernel::make< print< int > >() );

   raft::map.exe();

   return( EXIT_SUCCESS );
}
