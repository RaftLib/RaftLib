/**
 * This one tests the split kernel that distributes 
 * work, which is distinct from the operator '<='
 * which automatically connects and replicates 
 * multiple downstream ports.
 */
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
   /**
    * The split kernel has a single input port, and N
    * output ports, basically it just distributes work
    * based on some policy. The default is round-robin,
    * although we'll likely implement more in the future.
    */
   split s;
   print p;
   
   raft::map m;
   m += a >> s /** split kernel **/>> p;
   m.exe();

   return( EXIT_SUCCESS );
}
