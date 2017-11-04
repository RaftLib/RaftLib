/**
 * disconnectedGraph - check to ensure that the 
 * proper exception is thrown if the graph is not 
 * fully connected when exe is called.
 */

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
      input.addPort< A >( "input_a", "input_b", "fail" );
      output.addPort< C  >( "sum" );
   }
   
   IMPL_CLONE();

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
   using type_t = std::int64_t;
   using gen = raft::test::generate< type_t >;
   using sum = Sum< type_t, type_t, type_t >;
   using p_out = raft::print< type_t, '\n' >;
   gen a( count ), b( count );
   sum s;
   p_out print;

   raft::map m;
   m += a >> s[ "input_a" ];
   m += b >> s[ "input_b" ];
   m += s >> print;
   try
   {
       m.exe();
   }
   catch( PortUnconnectedException &ex )
   {
       std::cerr << ex.what() << "\n";
       exit( EXIT_SUCCESS );
   }
   
   return( EXIT_FAILURE );
}
