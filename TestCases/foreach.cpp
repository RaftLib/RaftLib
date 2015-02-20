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
      T data;
      input[ "in" ].pop( data );
      std::cout << data << "\n";
      return( raft::proceed );
   }
};

int
main( int argc, char **argv )
{
   int *arr = (int*) malloc( sizeof( int ) * 100 );
   for( int i( 0 ); i < 100; i++ )
   {
      arr[ i ] = i * 2;
   }


   raft::map.link( 
      raft::kernel::make< raft::for_each< int > >( arr, 100, 1),
      raft::kernel::make< print< int > >() );

   raft::map.exe();
   
   free( arr );
   return( EXIT_SUCCESS );
}
