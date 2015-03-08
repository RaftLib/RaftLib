#include <iostream>
#include <cstdlib>
#include <cstdint>
#include <vector>
#include <array>

#include "filter.tcc"
#include "stat.tcc"

int
main( int argc, char **argv )
{
   std::array< double, 6 > data = {{1.0, 1.0, 0.0, 1.0, 4.0, 2.0}};
   for( const auto val : data )
   {
      std::cout << val << " ";
   }
   std::cout << "\n";
   stat::scale_to_range( data, 5.23, 10 );
   for( const auto val : data )
   {
      std::cout << val << " ";
   }
   std::cout << "\n";
   return( EXIT_FAILURE );
}
