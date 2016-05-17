#include <iostream>
#include <cstdlib>
#include <cstdint>
#include <cassert>

#include "alloc_traits.tcc"

template < std::size_t N > struct varlen
{
   volatile char pad[ N ];
};

int
main()
{
   if( fits_in_cache_line< varlen< L1D_CACHE_LINE_SIZE > >::value != true )
   {
       std::cerr << "fail test\n";
       exit( EXIT_FAILURE );
   }
   return( EXIT_SUCCESS );
}
