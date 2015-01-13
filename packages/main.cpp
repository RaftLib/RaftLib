#include <iostream>
#include <cstdlib>
#include <cstdint>
#include <vector>
#include <array>

#include "filter.tcc"

int
main( int argc, char **argv )
{
   std::array< double, 6 > data = {{1, 1, 0, 1, 4, 2}};
   std::vector< double >   output;
   filter< double, 2, Gaussian > f;
   f.standardize;
   f.apply( data, output );
   for( const auto v : output )
   {
      std::cerr << "main: " << v << "\n";
   }
   return( EXIT_FAILURE );
}
