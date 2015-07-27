#include <cassert>
#include <iostream>
#include <cstdint>
#include <cstdlib>
#include <raft>
#include <raftio>
#include <raftrandom>
#include <raftmath>
#include <raftutility>

template< typename T  > class Sum : public raft::kernel
{
public:
   Sum() : raft::kernel()
   {
      input.template addPort< T >( "a" );
      input.template addPort< T >( "b" );
      output.template addPort< T  >( "sum" );
   }
   
   virtual raft::kstatus run()
   {
      auto &out( output[ "sum" ].template allocate< T >() );
      out = std::move( raft::sum< T >( input[ "a" ], input[ "b" ] ) );
      output[ "sum" ].send();
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
   using gen = raft::random_variate< type_t, raft::sequential >;
   using sum = Sum< type_t >;
   using p_out = raft::print< type_t, '\n' >;

   auto linked_kernels( 
      raft::map.link( raft::kernel::make< gen >( 1    /* min range */,
                                                 100  /* max range */, 
                                                 1    /* delta */,
                                                 count ),
                      raft::kernel::make< sum >(), "a" ) );
   raft::map.link( 
      raft::kernel::make< gen >( 1     /* min range */, 
                                 100   /* max range */,
                                 1     /* delta */, 
                                 count ),
      &linked_kernels.getDst(), "b"  );
   raft::map.link( 
      &linked_kernels.getDst(), 
      raft::kernel::make< p_out >() );
   raft::map.exe();
   return( EXIT_SUCCESS );
}
