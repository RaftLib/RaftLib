#include <iostream>
#include <cstdlib>
#include <cstdint>
#include <cassert>
#include <cstddef>
#include "alloc_traits.tcc"



template < std::size_t N > class foo
{
public:
   foo( int a ) : A( a ){}

private:
   int A;
   char __attribute__((__unused__)) pad[ N ];
};

int
main()
{
   assert( fits_in_cache_line< int[ 32 ] >::value == false );
   /** here's an array that should be externally allocated **/
   assert( ext_mem_alloc< int[ 32 ] >::value == true );
   /** lets make sure it fails with class types **/
   assert( ext_mem_alloc< foo< 100 > >::value == false );
   /** lets also make sure it fails with fundamental types **/
   assert( ext_mem_alloc< long double >::value == false );
   return( EXIT_SUCCESS );
}
