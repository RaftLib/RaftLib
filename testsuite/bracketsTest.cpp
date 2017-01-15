#include <cassert>
#include <iostream>
#include <cstdint>
#include <cstdlib>
#include <raft>
#include <raftio>

#include "generate.tcc" 


int
main()
{
   using type_t = std::int64_t;
   using gen = generate< type_t >;
   gen g( 1000 ) ;
   using p_out = raft::print< type_t, '\n' >;
   p_out print;
   
   raft::map m;
   m += g >> print;
   m.exe();

   return( EXIT_SUCCESS );
}
