#include <iostream>
#include <cstdlib>
#include <cstdint>
#include <cassert>
#include <cstddef>
#include "alloc_traits.tcc"
#include "defs.hpp"


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
   UNUSED( argc );
   if( fits_in_cache_line< int[ 32 ] >::value != false )
   {
       std::cerr << "test 1 (" << argv[ 0 ] << ") failed\n";
       exit( EXIT_FAILURE );
   }
   /** here's an array that should be externally allocated **/
   if( ext_mem_alloc< int[ 32 ] >::value != true )
   {
       std::cerr << "test 2 (" << argv[ 0 ] << ") failed\n";
       exit( EXIT_FAILURE );
   }
   /** lets make sure it fails with class types **/
   if( ext_mem_alloc< foo< 100 > >::value != false )
   {
       std::cerr << "test 3 (" << argv[ 0 ] << ") failed\n";
       exit( EXIT_FAILURE );
   }
   /** lets also make sure it fails with fundamental types **/
   if( ext_mem_alloc< long double >::value != false )
   {
       std::cerr << "test 4 (" << argv[ 0 ] << ") failed\n";
       exit( EXIT_FAILURE );
   }
   return( EXIT_SUCCESS );
}
