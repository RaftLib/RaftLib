#include <iostream>
#include <cstdlib>
#include <cstdint>
#include <vector>
#include <array>
#include "utility.hpp"

int
main( int argc, char **argv )
{
   const int f( 100 );
   int a( 4 );
   for( const auto n : raft::irange( 1,100 ) )
   {
      std::cerr << n << "\n";
   }
   return( EXIT_FAILURE );
}
