#include <cassert>
#include <iostream>
#include <cstdint>
#include <cstdlib>
#include <raft>
#include <raftio>
#include <raftrandom>


template< typename A, typename B, typename C > class sum : public raft::kernel
{
public:
   sum() : raft::kernel()
   {
      input. template addPort< A >( "input_a" );
      input. template addPort< B >( "input_b" );
      output.template addPort< C >( "sum" );
   }
   virtual raft::kstatus run()
   {
      A a;
      B b;
      input[ "input_a" ].pop( a );
      input[ "input_b" ].pop( b );
      /** allocate mem directly on queue **/
      auto c( output[ "sum" ].template allocate_s< C >() );
      (*c) = a + b;
      /** mem automatically freed upon scope exit **/
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
   using gen   = raft::random_variate< send_t, 
                                       raft::sequential >;
   using add = sum< send_t, send_t, send_t >;
   using p_out = raft::print< send_t, '\n' >;

   auto linked_kernels( 
      raft::map.link( raft::kernel::make< gen >( 1, count ),
                      raft::kernel::make< add >(), "input_a" ) );
   raft::map.link( 
      raft::kernel::make< gen >( 1, count ),
      &linked_kernels.getDst(), "input_b"  );
   raft::map.link( 
      &linked_kernels.getDst(), 
      raft::kernel::make< p_out >() );
   raft::map.exe();
   return( EXIT_SUCCESS );
}
