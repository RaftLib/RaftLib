#include <cstdint>
#include <iostream>
#include "rollinghash.tcc"
#include <cstdlib>

int 
main( int argc, char **argv )
{
   raft::rollinghash< char, 
                      raft::inf >  rh; 
   return( EXIT_SUCCESS );
}
