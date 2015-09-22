#include <iostream>
#include <cstdint>
#include <cstdlib>
#include "filter.tcc"

int
main( int argc, char **argv )
{
   using namespace raft;

   raft::filter< float, 3, LaplacianGaussian > filter( .5 ); 
   std::vector< float > a = {0.524109,0.00230536,0.548864,0.721324,0.0924731,0.311146,0.182434,0.660397,0.563678,0.880584};
   std::vector< float > filtered;
   filter.apply( a, filtered );
   for( auto val : filtered )
   {
      std::cout << val << ",";
   }
   exit( EXIT_SUCCESS );
}
