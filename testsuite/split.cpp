#include <raft>
#include <cstdint>
#include <iostream>
#include <raftio>
#include "generate.tcc"

int
main()
{
   using namespace raft;
   using type_t = std::int64_t;
   using gen = raft::test::generate< type_t >;
   using print = raft::print< type_t, '\n' >;
   using split = raft::split< type_t >;
   const auto count( 10000 );
   gen a( count );
   split s;
   print p;
   
   raft::map m;
   m += a >> s >> p;
   m.exe();

   return( EXIT_SUCCESS );
}
