#include <iostream>
#include <cstdlib>
#include <cstdint>
#include <vector>
#include <array>
#include "utility.hpp"

int
main( int argc, char **argv )
{
   for( const auto n : raft::range( 2.0,1.0,.1 ) )
   {
      std::cerr << n << "\n";
   }
   return( EXIT_FAILURE );
}
