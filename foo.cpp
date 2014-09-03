#include <cstdlib>
#include <iostream>
#include <vector>

int
main( int argc, char **argv )
{
   std::vector< std::vector * > foo;
   foo.push_back( new std::vector< int >() );
   foo.push_back( new std::vector< float >() );
   return( EXIT_SUCCESS );
}
