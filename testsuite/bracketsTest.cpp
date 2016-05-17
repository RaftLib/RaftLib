#include <cassert>
#include <iostream>
#include <cstdint>
#include <cstdlib>
#include <raft>
#include <raftio>


template < typename T > class Generate : public raft::kernel
{
public:
   Generate( T count = 1000 ) : raft::kernel(),
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
   T count;
};


int
main()
{
   using type_t = std::int64_t;
   using gen = Generate< type_t >;
   gen g( 1000 ) ;
   using p_out = raft::print< type_t, '\n' >;
   p_out print;
   
   raft::map m;
   m += g >> print;
   m.exe();

   return( EXIT_SUCCESS );
}
