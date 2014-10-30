#include <cassert>
#include <iostream>
#include <cstdint>
#include <cstdlib>
#include <raft>
#include <raftio>



template < typename T > class generate : public raft::kernel
{
public:
   generate( std::int64_t count = 1000000) : raft::kernel(),
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
   map.link( kernel::make< generate< std::int64_t > >(), 
             kernel::make< raft::print< std::int64_t, '\n' > >() );
   map.exe();
   return( EXIT_SUCCESS );
}
