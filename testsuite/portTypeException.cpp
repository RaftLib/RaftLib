#include <cassert>
#include <iostream>
#include <cstdint>
#include <cstdlib>
#include <raft>
#include <raftio>
#include <tuple>
#include "generate.tcc"
#include "defs.hpp"


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
      C c( static_cast< C >( a + b ) );
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
   raft::map m;
   gen g1( count ), g2( count );
   sum s;
   p_out p;
   
   auto linked_kernels( m += g1 >> s[ "input_a" ] );

   try
   {
        /** returns std::pair of iterators **/
        kernel_pair_t::kernel_iterator_type BEGIN, END;
        std::tie( BEGIN, END ) = linked_kernels.getDst();
        m += g2 >> (*BEGIN).get()[ "input_b" ];
   }
   catch( PortTypeMismatchException &ex )
   {
        UNUSED( ex );
        std::cout << ex.what() << "\n";
        //yippy, we threw the right exception
        return( EXIT_SUCCESS );
   }
   m += s >> p;
   m.exe();
   return( EXIT_FAILURE );
}
