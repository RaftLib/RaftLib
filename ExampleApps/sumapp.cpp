#include <cassert>
#include <iostream>
#include <cstdint>
#include <cstdlib>
#include <raft>
#include <raftio>

#include <vector>
#include <type_traits>
#include <utility>

namespace raft{
   Map map;
}

template < typename T > class generate : public raft::kernel
{
public:
   generate( std::int64_t count = 1000 ) : raft::kernel(),
                                           count( count )
   {
      output.addPort< T >( "number_stream" );
   }

   virtual raft::kstatus run()
   {
      if( count-- > 1 )
      {
         output[ "number_stream" ].push( count );
         return( raft::proceed );
      }
      output[ "number_stream" ].push( count, raft::eof );
      return( raft::stop );
   }

private:
   std::int64_t count;
};

template< typename A, typename B, typename C > class sum : public raft::kernel
{
public:
   sum() : raft::kernel()
   {
      input.addPort< A >( "input_a" );
      input.addPort< B >( "input_b" );
      output.addPort< C >( "sum" );
   }
#define M1 1 
   virtual raft::kstatus run()
   {
#ifdef M1 
      A a;
      B b;
      /** copies a & b **/
      input[ "input_a" ].pop( a );
      input[ "input_b" ].pop( b );
      C c = a + b;
      output[ "sum" ].push( c );
#elif defined M2 
      A a;
      B b;
      /** copies a & b **/
      input[ "input_a" ].pop( a );
      input[ "input_b" ].pop( b );
      /** allocate directly on queue **/
      auto &c( output[ "sum" ].allocate< C >() );
      c = a + b;
      output[ "sum" ].push();
#elif defined M3
      /** look at mem on head of queue for a & b, no copy **/
      auto a( input[ "input_a" ].pop_s< A >() );
      auto b( input[ "input_b" ].pop_s< B >() );
      /** allocate mem directly on queue **/
      auto c( output[ "sum" ].allocate_s< C >() );
      (*c) = (*a) + (*b);
      /** mem automatically freed upon scope exit **/
#endif      
      return( raft::proceed );
   }

};


int
main( int argc, char **argv )
{
   using namespace raft;
   const std::size_t count( 100000 );
   auto linked_kernels( 
      map.link( kernel::make< generate< std::int64_t > >( count ),
                kernel::make< sum< std::int64_t,std::int64_t, std::int64_t > >(),
                  "input_a" ) );
   map.link( kernel::make< generate< std::int64_t > >( count ), 
             &( linked_kernels.dst ), 
               "input_b" );
   map.link( &( linked_kernels.dst ), 
             kernel::make< raft::print< std::int64_t ,'\n'> >() );
   map.exe();
   return( EXIT_SUCCESS );
}
