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
      /** look at mem on head of queue for a & b, no copy **/
      auto a( input[ "input_a" ].pop_s< A >() );
      auto b( input[ "input_b" ].pop_s< B >() );
      /** allocate mem directly on queue **/
      auto c( output[ "sum" ].allocate_s< C >() );
      (*c) = (*a) + (*b);
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
   auto gen( raft::kernel::make< Generate< std::int64_t > >( count ) );
   
   auto linked_kernels( raft::map.link( gen->clone(),
                                        new sum< std::int64_t,std::int64_t, std::int64_t >(),
                                        "input_a" ) );
   raft::map.link( gen->clone() , &( linked_kernels.dst ), "input_b" );
   raft::map.link( &( linked_kernels.dst ), 
                   new raft::print< std::int64_t ,'\n' >() );
   raft::map.exe();
   return( EXIT_SUCCESS );
}
