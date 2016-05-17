#include <iostream>
#include <cstdlib>
#include <cstdint>
#include <cassert>

#include "alloc_traits.tcc"

template < std::size_t N > struct varlen
{
   char __attribute__((__unused__)) pad[ N ];
};

int
main()
{
   assert( fits_in_cache_line< varlen< L1D_CACHE_LINE_SIZE > >::value );
   return( EXIT_SUCCESS );
}
