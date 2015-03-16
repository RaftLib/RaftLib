#include <cassert>
#include <iostream>
#include <cstdint>
#include <cstdlib>
#include <raft>


template< typename A, typename B, typename C > class sum : public raft::kernel
{
public:
   Sum() : raft::kernel()
   {
      input.addPort< A >( "input_a" );
      input.addPort< B >( "input_b" );
      output.addPort< C  >( "sum" );
   }
   
   virtual raft::kstatus run()
   {
      A a;
      B b;
      raft::signal  sig_a( raft::none  ), sig_b( raft::none );
      input[ "input_a" ].pop( a, &sig_a );
      input[ "input_b" ].pop( b, &sig_b );
      assert( sig_a == sig_b );
      C c( a + b );
      output[ "sum" ].push( c , sig_a );
      if( sig_b == raft::eof )
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
   Map map;
   auto linked_kernels( map.link( new Generate< float >(),
                                  new Sum< std::int64_t,std::int64_t, std::int64_t >(),
                                  "input_a" ) );
   map.link( new Generate< std::int64_t >(), &( linked_kernels.dst ), "input_b" );
   map.link( &( linked_kernels.dst ), new Print< std::int64_t >() );
   map.exe();
   return( EXIT_SUCCESS );
}
