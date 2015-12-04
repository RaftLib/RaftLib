#include <cassert>
#include <iostream>
#include <cstdint>
#include <cstdlib>
#include <raft>
#include <raftio>
#include "generate.tcc"



template< typename A, typename B, typename C > class Sum : public raft::kernel
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
   int count( 1000 );
   if( argc == 2 )
   {
      count = atoi( argv[ 1 ] );
   }
   using send_t = std::int64_t;
   using wrong_t = float;
   using gen   = raft::test::generate< send_t >;
   using sum = Sum< send_t, 
                    wrong_t, 
                    send_t >;
   using p_out = raft::print< send_t, '\n' >;
   auto linked_kernels( 
      raft::map.link( raft::kernel::make< gen >( count ),
                      raft::kernel::make< sum >(), "input_a" ) );
   try{
   raft::map.link( 
      raft::kernel::make< gen >( count ),
      &linked_kernels.getDst(), "input_b"  );
   }
   catch( PortTypeMismatchException &ex )
   {
    //yippy, we threw the right exception
    return( EXIT_SUCCESS );
   }
   raft::map.link( 
      &linked_kernels.getDst(), 
      raft::kernel::make< p_out >() );
    raft::map.exe();
   return( EXIT_FAILURE );
}
