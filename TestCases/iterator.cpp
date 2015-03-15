#include <cassert>
#include <iostream>
#include <cstdint>
#include <cstdlib>
#include <raft>
#include <raftio>

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

template< typename A, typename C > class sum : public raft::kernel
{
public:
   sum() : raft::kernel()
   {
      input.addPort< A >( "input_a" );
      input.addPort< A >( "input_b" );
      output.addPort< C  >( "sum" );
   }
   
   virtual raft::kstatus run()
   {
      auto c( output[ "sum" ].allocate_s< C >() );
      (*c) = 0;
      for( auto &port : input )
      {
         A a;
         port.pop( a );
         (*c) += a;
      }
      return( raft::proceed );
   }

};

int
main( int argc, char **argv )
{
   const auto count( 10000 );
   using gen    = generate< std::int64_t >;
   using s      = sum< std::int64_t, std::int64_t >;
   using print  = raft::print< std::int64_t, '\n'>;
   auto kernels = 
      raft::map.link( 
         raft::kernel::make< gen >( count ),
         raft::kernel::make< s >(), "input_a" );
   raft::map.link(   
      raft::kernel::make< gen >( count ),
      &kernels.getDst(), "input_b" );
   raft::map.link( 
      &kernels.getDst(),
      raft::kernel::make< print >() );
   raft::map.exe();
   return( EXIT_SUCCESS );
}
