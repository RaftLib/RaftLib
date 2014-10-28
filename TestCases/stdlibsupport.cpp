#include <cassert>
#include <iostream>
#include <cstdint>
#include <cstdlib>
#include <vector>
#include <raft>


template< typename T > class print : public Kernel
{
public:
   print() : Kernel()
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
   using namespace raft;
   std::vector< std::uint32_t > v;
   int i( 0 );
   auto func( [&](){ return( i++ ); } );
   while( i < 1000 ){ v.push_back( func() ); }
   std::vector< std::uint32_t > o;
   /** link iterator reader to print kernel **/
   map.link( new read_container< std::uint32_t >( v.begin(), v.end() ),
             new write_container< std::uint32_t >( std::back_inserter( o ) ) );
   map.exe();
   /** once function returns, o should be readable **/
   for( auto val : o )
   {
      std::cout << val << "\n";
   }
   return( EXIT_SUCCESS );
}
