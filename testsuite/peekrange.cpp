#include <cassert>
#include <iostream>
#include <cstdint>
#include <cstdlib>
#include <vector>
#include <iterator>
#include <raft>
#include <raftio>

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


#define COUNT 20

int
main()
{
   int *arr( 
    static_cast< int* >( malloc( sizeof( int ) * COUNT ) )
   );
   for( auto i( 0 ); i < COUNT; i++ )
   {
      arr[ i ] = i;
   }
   
   raft::map m;
   raft::for_each< int > fe( arr, COUNT, 1 );
   print< int > pr;
   m += fe >> pr;
   m.exe();
   return( EXIT_SUCCESS );
}
