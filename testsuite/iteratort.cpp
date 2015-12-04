#include <cassert>
#include <iostream>
#include <cstdint>
#include <cstdlib>
#include <raft>
#include <raftio>
#include "generate.tcc"

template< typename A, typename C > class sum : public raft::kernel
{
public:
   sum() : raft::kernel()
   {
      input.addPort< A >( "input_a" );
      input.addPort< A >( "input_b" );
      output.addPort< C  >( "sum" );
   }
   
   virtual raft::kstatus run()
   {
      auto c( output[ "sum" ].template allocate_s< C >() );
      (*c) = 0;
      for( auto &port : input )
      {
         A a;
         port.pop( a );
         (*c) += a;
      }
      return( raft::proceed );
   }

};

int
main( int argc, char **argv )
{
   const auto count( 10000 );
   using send_t = std::int64_t;
   using gen   = raft::test::generate< send_t >;
   
   using s      = sum< send_t, send_t >;
   using print  = raft::print< send_t, '\n'>;
   auto kernels = 
      raft::map.link( 
         raft::kernel::make< gen >( count ),
         raft::kernel::make< s >(), "input_a" );
   raft::map.link(   
      raft::kernel::make< gen >( count ),
      &kernels.getDst(), "input_b" );
   raft::map.link( 
      &kernels.getDst(),
      raft::kernel::make< print >() );
   raft::map.exe();
   return( EXIT_SUCCESS );
}
