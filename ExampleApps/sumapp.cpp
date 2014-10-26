#include <cassert>
#include <iostream>
#include <cstdint>
#include <cstdlib>
#include <raft>
#include <vector>
#include <type_traits>
#include <utility>

#include "print.tcc"

template < typename T > class Generate : public Kernel
{
public:
   Generate( std::int64_t count = 1000 ) : Kernel(),
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

template< typename A, typename B, typename C > class Sum : public Kernel
{
public:
   Sum() : Kernel()
   {
      input.addPort< A >( "input_a" );
      input.addPort< B >( "input_b" );
      output.addPort< C >( "sum" );
   }
   
   virtual raft::kstatus run()
   {
      //A a;
      //B b;
      //input[ "input_a" ].pop( a );
      //input[ "input_b" ].pop( b );
      /** there's a bug in the allocate code */
      //auto &c( output[ "sum" ].allocate< C >() );
      //c = a + b;
      //C c = a + b;
      //output[ "sum" ].push( c );
      auto a( input[ "input_a" ].template pop_s< A >() );
      auto b( input[ "input_b" ].template pop_s< B >() );
      auto c( output[ "sum" ].template allocate_s< C >() );
      (*c) = (*a) + (*b);
      return( raft::proceed );
   }

};


int
main( int argc, char **argv )
{
   using namespace raft;
   const std::size_t count( 100000 );
   auto linked_kernels( 
      map.link( new Generate< std::int64_t >( count ),
                new Sum< std::int64_t,std::int64_t, std::int64_t >(),
                  "input_a" ) );
   map.link( new Generate< std::int64_t >( count ), 
             &( linked_kernels.dst ), 
               "input_b" );
   map.link( &( linked_kernels.dst ), 
             new Print< std::int64_t ,'\n'>() );
   map.exe();
   return( EXIT_SUCCESS );
}
