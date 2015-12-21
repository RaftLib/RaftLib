#include <cassert>
#include <iostream>
#include <cstdint>
#include <cstdlib>
#include <raft>
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
         
         auto &ref( output[ "number_stream" ].template allocate< T >() );
         ref = count;
         output[ "number_stream"].send();
         
         return( raft::proceed );
      }
      /** else **/
      auto &ref( output[ "number_stream" ].template allocate< T >() );
      ref = count;
      output[ "number_stream" ].send( raft::eof );
      return( raft::stop );
   }

private:
   std::int64_t count;
};


int
main( int argc, char **argv )
{
   using gen = Generate< std::int64_t >;
   gen g;
   g[ "a" ];
   g[ "b" ];
   //should throw exception here
   try
   {
        g[ "c" ];
   }
   catch( AmbiguousPortAssignmentException &ex )
   {
     std::cerr << ex.what() << "\n";
     exit( EXIT_SUCCESS );
   }
   //otherwise, failure 
   return( EXIT_FAILURE );
}
