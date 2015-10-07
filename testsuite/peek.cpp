#include <cassert>
#include <iostream>
#include <cstdint>
#include <cstdlib>
#define PEEKTEST 1
#include <raft>
#include <raftio>


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
         
         auto &ref( output[ "number_stream" ].template allocate< T >() );
         ref = count;
         output[ "number_stream"].send();
         
         return( raft::proceed );
      }
      /** else **/
      auto &ref( output[ "number_stream" ].template allocate< T >() );
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
      auto &a( input[ "input_a" ].template peek< A >() );
      auto &b( input[ "input_b" ].template peek< B >() );
      C c( a + b );
      /** force unpeek error, won't resize **/
      input[ "input_a" ].unpeek();
      input[ "input_a" ].recycle( a );
      input[ "input_b" ].recycle( b );
      output[ "sum" ].push( c );
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
   using gen = Generate< std::int64_t >;
   using sum = Sum< std::int64_t, std::int64_t, std::int64_t >;
   using p_out = raft::print< std::int64_t, '\n' >;

   auto linked_kernels( 
      raft::map.link( raft::kernel::make< gen >( count ),
                      raft::kernel::make< sum >(), "input_a" ) );
   raft::map.link( 
      raft::kernel::make< gen >( count ),
      &linked_kernels.getDst(), "input_b"  );
   raft::map.link( 
      &linked_kernels.getDst(), 
      raft::kernel::make< p_out >() );
   raft::map.exe();
   return( EXIT_SUCCESS );
}
