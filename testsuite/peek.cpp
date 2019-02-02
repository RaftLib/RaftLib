#include <cassert>
#include <iostream>
#include <cstdint>
#include <cstdlib>
#define PEEKTEST 1
#include <raft>
#include <raftio>
#include <raftmath>
#include "generate.tcc"



template < typename T > class Sum : public raft::kernel
{
public:
   Sum() : raft::kernel()
   {
      input.addPort<  T > ( "input_a" );
      input.addPort<  T > ( "input_b" );
      output.addPort< T  >( "sum"     );
   }
   
   IMPL_CLONE();

   virtual raft::kstatus run()
   {
      output[ "sum" ].push( raft::sum< T >( input[ "input_a" ], 
                                            input[ "input_b" ] ) );
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
   using sum = Sum< type_t >;
   using print  = raft::print< type_t, '\n' >;

   gen a( count ), b( count );
   sum s;
   print p;
   raft::map m;
   m += a >> s[ "input_a" ];
   m += b >> s[ "input_b" ];
   m += s >> p;
   m.exe();
   return( EXIT_SUCCESS );
}
