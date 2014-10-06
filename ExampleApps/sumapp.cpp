#include <cassert>
#include <iostream>
#include <cstdint>
#include <cstdlib>
#include <raft>
#include <vector>

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
      std::vector< T > list;
      while( count-- ) list.push_back( count );
      output[ "number_stream" ].insert( list.begin(),
                                        list.end(),
                                        raft::eof );
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
      C &c( output[ "sum" ].allocate< C >( ) );
      c = ( a + b );
      output[ "sum" ].push( sig_a );
      if( sig_a == raft::eof || sig_b == raft::eof )
      {
         return( raft::stop );
      }
      return( raft::proceed );
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
