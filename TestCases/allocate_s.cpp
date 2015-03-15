#include <cassert>
#include <iostream>
#include <cstdint>
#include <cstdlib>
#include <raft>
#include <raftio>

template < typename T > class Generate : public raft::kernel
{
public:
   Generate( std::int64_t count = 10 ) : raft::kernel(),
                                           count( count )
   {
      output.addPort< T >( "number_stream" );
   }

   Generate( const Generate< T > &other ) : raft::kernel(),
                                            count( other.count )
   {
      output.addPort< T >( "number_stream" );
   }

   CLONE();

   virtual raft::kstatus run()
   {
      if( count-- > 1 )
      {
         auto ref( output[ "number_stream" ].allocate_s< T >() );
         (*ref) = count;
         return( raft::proceed );
      }
      /** else **/
      auto ref( output[ "number_stream" ].allocate_s< T >() );
      (*ref) = count;
      ref.sig() = raft::eof;
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
   virtual raft::kstatus run()
   {
      A a;
      B b;
      input[ "input_a" ].pop( a );
      input[ "input_b" ].pop( b );
      /** allocate mem directly on queue **/
      auto c( output[ "sum" ].allocate_s< C >() );
      (*c) = a + b;
      /** mem automatically freed upon scope exit **/
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
   using add = sum< std::int64_t, std::int64_t, std::int64_t >;
   using p_out = raft::print< std::int64_t, '\n' >;

   auto linked_kernels( 
      raft::map.link( raft::kernel::make< gen >( count ),
                      raft::kernel::make< add >(), "input_a" ) );
   raft::map.link( 
      raft::kernel::make< gen >( count ),
      &linked_kernels.getDst(), "input_b"  );
   raft::map.link( 
      &linked_kernels.getDst(), 
      raft::kernel::make< p_out >() );
   raft::map.exe();
   return( EXIT_SUCCESS );
}
