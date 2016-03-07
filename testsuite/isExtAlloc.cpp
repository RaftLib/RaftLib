#include <iostream>
#include <cstdlib>
#include <cstdint>
#include <cassert>
#include <cstddef>
#include <array>
#include "alloc_traits.tcc"



template < std::size_t N > class foo
{
public:
   foo( int a ) : A( a ){}

private:
   int A;
   char pad[ N ];
};

int
main( int argc, char **argv )
{
   assert( fits_in_cache_line< int[ 32 ] >::value == false );
   /** true cases **/
   assert( ext_alloc< int[32] >::value );
   assert( ext_alloc< foo< 100 > >::value );
   std::array< int, 100 > f;
   assert( ext_alloc< decltype( f ) >::value );
   /** false cases **/
   assert( ext_alloc< float >::value == false );
   assert( ext_alloc< int >::value == false );
   assert( ext_alloc< int[16] >::value == false );
   assert( ext_alloc< foo< 60 > >::value == false );
   return( EXIT_SUCCESS );
}
