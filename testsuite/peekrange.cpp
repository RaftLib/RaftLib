#include <cassert>
#include <iostream>
#include <cstdint>
#include <cstdlib>
#include <vector>
#include <iterator>
#include <raft>
#include <raftio>
#include "defs.hpp"

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
      /**
       * NOTE: preferably this would be a constant ref
       * however, with qthreads the stack frames can behave
       * oddly when unwinding so..keep as copy for now, the
       * /dev branch has a totally different code base backing
       * exceptions which  fixes the issue. -jcb 1 July 2017
       */
      catch( NoMoreDataException &ex )
      {
         std::cerr << ex.what() << "\n";
         /** nothing bad, just no more data **/
         return( raft::stop );
      }
      catch( ClosedPortAccessException &ex )
      {
         UNUSED( ex );
         /** nothing bad, just no more data **/
         return( raft::stop );
      }
      std::cout << "\n";
      port.recycle( 2 );
      return( raft::proceed );
   }
};


#define COUNT 7

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
