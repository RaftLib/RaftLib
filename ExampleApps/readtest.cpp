#include <cstdlib>
#include <iostream>
#include <raft>

#include "fileio.tcc"
#include "print.tcc"

int
main( int argc, char **argv )
{
   using namespace raft;
   map.link( new filereader<filechunk< 1024  >,false>( "ecoli.txt" ), 
             new Print< filechunk< 1024  > >() );

   map.exe();
   return( EXIT_SUCCESS );
};
