#include <iostream>
#include <cstdlib>
#include "kernel.hpp"


template < typename T > class Generate : public Kernel
{
public:
   Generate( std::int64_t count = 100 ) : Kernel(),
                                          count( count )
   {
      output.addPort< T >( "number_stream" );
   }

   virtual void run()
   {
      while( count-- > 1 )
      {
         output[ "number_stream" ].push( count );
      }
      output[ "number_stream" ].push( count, RBSignal::RBEOF );
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
   };
   
   virtual void run()
   {
      A a;
      B b;
      RBSignal sig_a( RBSignal::NONE ), sig_b( RBSignal::NONE );
      input[ "input_a" ].pop( a, &sig_a );
      input[ "input_b" ].pop( b, &sig_b );
      output[ "sum" ].push< decltype( A + B ) >( ( a + b ), sig_a );
   }

};

template< typename T > class Print : public Kernel
{
public:
   Print() : Kernel()
   {
      input.addPort< T >( "in" );
   }

   virtual void run()
   {
      T data;
      input[ "in" ].pop( 
   }
};
