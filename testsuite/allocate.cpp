#include <cassert>
#include <iostream>
#include <cstdint>
#include <cstdlib>
#include <raft>
#include <raftio>
//#define STRING_NAMES 1
#include "generate.tcc"


#ifndef STRING_NAMES
template< typename A, typename B, typename C > class Sum : public raft::kernel
{
public:
   Sum() : raft::kernel()
   {
      input.addPort< A >( "input_a"_port );
      input.addPort< B >( "input_b"_port );
      output.addPort< C  >( "sum"_port );
   }
   
   virtual raft::kstatus run()
   {
      A a;
      B b;
      raft::signal  sig_a( raft::none  ), sig_b( raft::none );
      input[ "input_a"_port ].pop( a, &sig_a );
      input[ "input_b"_port ].pop( b, &sig_b );
      assert( sig_a == sig_b );
      C c( a + b );
      output[ "sum"_port ].push( c , sig_a );
      if( sig_b == raft::eof )
      {
         return( raft::stop );
      }
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
    
    //int64
    {
        using type_t = std::int64_t;
        using gen = raft::test::generate< type_t >;
        using sum = Sum< type_t, type_t, type_t >;
        using p_out = raft::print< type_t, '\n' >;
        gen a( count ), b( count );
        sum s;
        p_out print;

        raft::map m;
        m += a >> s[ "input_a"_port ];
        m += b >> s[ "input_b"_port ];
        m += s >> print;
        m.exe();
    }
    
    //bool
    {
        using type_t = bool;
        using gen = raft::test::generate< type_t >;
        using sum = Sum< type_t, type_t, type_t >;
        using p_out = raft::print< type_t, '\n' >;
        gen a( count ), b( count );
        sum s;
        p_out print;

        raft::map m;
        m += a >> s[ "input_a"_port ];
        m += b >> s[ "input_b"_port ];
        m += s >> print;
        m.exe();
    }
    //std::string
    {
        using type_t = std::string;
        using gen = raft::test::generate< type_t >;
        using sum = Sum< type_t, type_t, type_t >;
        using p_out = raft::print< type_t, '\n' >;
        gen a( count ), b( count );
        sum s;
        p_out print;

        raft::map m;
        m += a >> s[ "input_a"_port ];
        m += b >> s[ "input_b"_port ];
        m += s >> print;
        m.exe();
    }

    return( EXIT_SUCCESS );
}
#else
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

int
main( int argc, char **argv )
{
    int count( 1000 );
    if( argc == 2 )
    {
       count = atoi( argv[ 1 ] );
    }
    
    //int64
    {
        using type_t = std::int64_t;
        using gen = raft::test::generate< type_t >;
        using sum = Sum< type_t, type_t, type_t >;
        using p_out = raft::print< type_t, '\n' >;
        gen a( count ), b( count );
        sum s;
        p_out print;

        raft::map m;
        m += a >> s[ "input_a" ];
        m += b >> s[ "input_b" ];
        m += s >> print;
        m.exe();
    }
    
    //bool
    {
        using type_t = bool;
        using gen = raft::test::generate< type_t >;
        using sum = Sum< type_t, type_t, type_t >;
        using p_out = raft::print< type_t, '\n' >;
        gen a( count ), b( count );
        sum s;
        p_out print;

        raft::map m;
        m += a >> s[ "input_a" ];
        m += b >> s[ "input_b" ];
        m += s >> print;
        m.exe();
    }
    //std::string
    {
        using type_t = std::string;
        using gen = raft::test::generate< type_t >;
        using sum = Sum< type_t, type_t, type_t >;
        using p_out = raft::print< type_t, '\n' >;
        gen a( count ), b( count );
        sum s;
        p_out print;

        raft::map m;
        m += a >> s[ "input_a" ];
        m += b >> s[ "input_b" ];
        m += s >> print;
        m.exe();
    }

    return( EXIT_SUCCESS );
}
#endif
