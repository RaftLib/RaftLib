#include <cassert>
#include <iostream>
#include <cstdint>
#include <cstdlib>

#include "map.hpp"
#include "kernel.hpp"


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
      output[ "number_stream" ].push( count, RBSignal::RBEOF );
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
      RBSignal sig_a( RBSignal::NONE ), sig_b( RBSignal::NONE );
      input[ "input_a" ].pop( a, &sig_a );
      input[ "input_b" ].pop( b, &sig_b );
      C c( a + b );
      output[ "sum" ].push( c , sig_a );
      assert( sig_a == sig_b );
      if( sig_b == RBSignal::RBEOF )
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
      RBSignal signal( RBSignal::NONE );
      input[ "in" ].pop( data, &signal );
      fprintf( stderr, "%" PRIu64 "\n", data );
      if( signal == RBSignal::RBEOF )
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
   auto *sum( new Sum< std::int64_t,std::int64_t, std::int64_t >() );
   map.link( new Generate< std::int64_t >(),
             sum,
             "input_a" );
   map.link( new Generate< std::int64_t >(), sum, "input_b" );
   map.link( sum, new Print< std::int64_t >() );
   map.exe();
   return( EXIT_SUCCESS );
}
