#include <cassert>
#include <iostream>
#include <cstdint>
#include <cstdlib>
#include <vector>
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
   using namespace raft;
   std::vector< std::uint32_t > v;
   int i( 0 );
   auto func( [&](){ return( i++ ); } );
   while( i < 1000 ){ v.push_back( func() ); }
   std::vector< std::uint32_t > o;
   /** link iterator reader to print kernel **/
   map.link( kernel::make< raft::read_each< std::uint32_t > >( v.begin(), v.end() ),
             kernel::make< raft::write_each< std::uint32_t > >( std::back_inserter( o ) ) );
   
   //map.link( new read_each< std::uint32_t >( v.begin(), v.end() ),
   //          new write_each< std::uint32_t >( std::back_inserter( o ) ) );
   map.exe();
   /** once function returns, o should be readable **/
   for( auto val : o )
   {
      std::cout << val << "\n";
   }
   return( EXIT_SUCCESS );
}
