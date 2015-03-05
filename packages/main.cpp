#include <iostream>
#include <cstdlib>
#include <cstdint>
#include <vector>
#include <array>

#include "filter.tcc"

int
main( int argc, char **argv )
{
   std::array< double, 6 > data = {{1.0, 1.0, 0.0, 1.0, 4.0, 2.0}};
   std::vector< double >   output;
   filter< double, 1, Gaussian > f;
   f.apply( data, output );
   std::cerr << "Gaussian Filter: \n";
   for( const auto v : output )
   {
      std::cerr  << v << " ";
   }
   std::cerr << "\n\n";
   std::vector< double > l_output;
   filter< double, 1, LaplacianGaussian > l( .5 );
   l.apply( data, l_output );
   std::cerr << "Laplacian: \n";
   for( const auto v : l_output )
   {
      std::cerr << v << " ";
   }
   std::cerr << "\n\n";

   return( EXIT_FAILURE );
}
