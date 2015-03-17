#include <raft>
#include <cstdint>
#include <iostream>
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
         output[ "number_stream" ].push( count );
         return( raft::proceed );
      }
      output[ "number_stream" ].push( count, raft::eof );
      return( raft::stop );
   }

private:
   std::int64_t count;
};

int
main( int argc, char **argv )
{
   using namespace raft;
   Map map;
   /** manually link split kernels **/
   auto kernels( 
   map.link( kernel::make< Generate< std::int64_t > >(),
             kernel::make< split< std::int64_t > >() ) );
   
   map.link( &kernels.getDst(), 
             kernel::make< print< std::int64_t, '\n' > >() );
   map.exe();
   return( EXIT_FAILURE );
}
