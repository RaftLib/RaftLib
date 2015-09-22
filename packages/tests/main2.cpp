#include <cstdint>
#include <iostream>
#include <cstdlib>
#include "streamingstat.tcc"
#include "utility.hpp"

int 
main( int argc, char **argv )
{
   raft::streamingstat< double > stat;
   for( const auto &val : raft::range( -1.0, 1.0, .001 ) )
   {
      stat.update( val );
   }
   std::cout << "Mean: " << stat.mean< float >() << "\n";
   std::cout << "STD: " << stat.std< float >() << "\n";
   return( EXIT_SUCCESS );
}
