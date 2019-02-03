/**
 * simple test case to try out allocation of strings. This 
 * appears to fail on some configurations with Windows 10,
 * haven't quite figured out the cause yet but will update
 * this note when we do.
 */

#include <cassert>
#include <iostream>
#include <cstdint>
#include <cstdlib>
#include <sstream>
#include <raft>
#include <raftio>
#include "generate.tcc"

template< typename T > class sum : public raft::kernel
{
public:
   sum() : raft::kernel()
   {
      input. template addPort< T >( "input_a", "input_b" );
      output.template addPort< std::string >( "sum" );
   }

   IMPL_NO_CLONE();

   virtual raft::kstatus run()
   {
      T a,b; 
      input[ "input_a" ].pop( a );
      input[ "input_b" ].pop( b );
      /** allocate mem directly on queue **/
      auto c( output[ "sum" ].template allocate_s< std::string >() );
      std::stringstream ss;
      ss << a << " + " << b << " = " << (a + b);
      *c = ss.str();
      /** mem automatically freed upon scope exit **/
      return( raft::proceed );
   }

};

int
main( int argc, char **argv )
{
   int count( 10000 );
   if( argc == 2 )
   {
      count = atoi( argv[ 1 ] );
   }
   
   using send_t = std::int64_t;
   using gen   = raft::test::generate< send_t >;

   using add = sum< send_t >;
   using p_out = raft::print< std::string , '\n' >;
   
   gen a( count ), b( count );
   add s;
   p_out print;

   raft::map m;
   m += a >> s[ "input_a" ];
   m += b >> s[ "input_b" ];
   m += s >> print;
   m.exe();
   
   return( EXIT_SUCCESS );
}
