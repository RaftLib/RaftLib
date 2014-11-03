#include <cstdlib>
#include <iostream>
#include <raft>
#include <raftio>

namespace raft
{
   Map map;
}

int
main( int argc, char **argv )
{
   using namespace raft;
   
   map.link( kernel::make< raft::filereader<filechunk< 1024  >, false> > ( "ecoli.txt" ), 
             kernel::make< raft::print< filechunk< 1024  > > >() );

   map.exe();
   return( EXIT_SUCCESS );
};
