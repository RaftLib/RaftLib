#include <cassert>
#include <iostream>
#include <cstdint>
#include <cstdlib>
#include <raft>


template < typename T > class Generate : public Kernel
{
public:
   Generate( std::int64_t count = 100 ) : Kernel(),
                                          count( count )
   {
      output.addPort< T >( "number_stream" );
   }

   virtual bool run()
   {
      if( count-- > 1 )
      {
         output[ "number_stream" ].push( count );
         return( true );
      }
      /** else **/
      output[ "number_stream" ].push( count, raft::eof );
      return( false );
   }

private:
   std::int64_t count;
};

template< typename A, typename B, typename C > class Sum : public Kernel
{
public:
   Sum() : Kernel()
   {
      input.addPort< A >( "input_a" );
      input.addPort< B >( "input_b" );
      output.addPort< C  >( "sum" );
   }
   
   virtual bool run()
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
         return( false );
      }
      return( true );
   }

};

template< typename T > class Print : public Kernel
{
public:
   Print() : Kernel()
   {
      input.addPort< T >( "in" );
   }

   virtual bool run()
   {
      T data;
      raft::signal  signal( raft::none );
      input[ "in" ].pop( data, &signal );
      fprintf( stderr, "%" PRIu64 "\n", data );
      if( signal == raft::eof )
      {
         return( false );
      }
      return( true );
   }
};

int
main( int argc, char **argv )
{
   Map map;
   auto linked_kernels( map.link( new Generate< std::int64_t >(),
                                  new Sum< std::int64_t,std::int64_t, std::int64_t >(),
                                  "input_a" ) );
   map.link( new Generate< std::int64_t >(), &( linked_kernels.dst ), "input_b" );
   map.link( &( linked_kernels.dst ), new Print< std::int64_t >() );
   map.exe();
   return( EXIT_SUCCESS );
}
