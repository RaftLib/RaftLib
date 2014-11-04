#include <cassert>
#include <iostream>
#include <cstdint>
#include <cstdlib>
#include <raft>


template < typename T > class Generate : public raft::kernel
{
public:
   Generate( std::int64_t count = 1000 ) : raft::kernel(),
                                           count( count )
   {
      output.addPort< T >( "number_stream" );
   }

   virtual raft::kstatus run()
   {
      if( count-- > 1 )
      {
         
         auto &ref( output[ "number_stream" ].allocate< T >() );
         ref = count;
         output[ "number_stream"].send();
         
         return( raft::proceed );
      }
      /** else **/
      auto &ref( output[ "number_stream" ].allocate< T >() );
      ref = count;
      output[ "number_stream" ].send( raft::eof );
      return( raft::stop );
   }

private:
   std::int64_t count;
};

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

template< typename T > class Print : public raft::kernel
{
public:
   Print() : raft::kernel()
   {
      input.addPort< T >( "in" );
   }

   virtual raft::kstatus run()
   {
      T data;
      raft::signal  signal( raft::none );
      input[ "in" ].pop( data, &signal );
      fprintf( stderr, "%" PRIu64 "\n", data );
      if( signal == raft::eof )
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
   auto linked_kernels( map.link( new Generate< std::int64_t >(),
                                  new Sum< std::int64_t,std::int64_t, std::int64_t >(),
                                  "input_a" ) );
   map.link( new Generate< std::int64_t >(), &( linked_kernels.dst ), "input_b" );
   map.link( &( linked_kernels.dst ), new Print< std::int64_t >() );
   map.exe();
   return( EXIT_SUCCESS );
}
