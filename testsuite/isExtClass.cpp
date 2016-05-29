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
   if( fits_in_cache_line< foo< 4 > >::value != true )
   {
       std::cerr << "test: " << argv[ 0 ] << " failed\n";
       exit( EXIT_FAILURE );
   }
   /** should be inline class allocate **/
   if( inline_class_alloc< foo< 4 > >::value != true )
   {
       std::cerr << "test: " << argv[ 0 ] << " failed\n";
       exit( EXIT_FAILURE );
   }
   /** should be fundamental type, and be false **/
   if( inline_class_alloc< int >::value != false )
   {
       std::cerr << "test: " << argv[ 0 ] << " failed\n";
       exit( EXIT_FAILURE );
   }
   /** should be too big, ret false **/
   if( inline_class_alloc< foo< 128 > >::value != false )
   {
       std::cerr << "test: " << argv[ 0 ] << " failed\n";
       exit( EXIT_FAILURE );
   }
   /** should be false, not a class **/
   if( inline_class_alloc< int[ 2 ] >::value != false )
   {
       std::cerr << "test: " << argv[ 0 ] << " failed\n";
       exit( EXIT_FAILURE );
   }
   return( EXIT_SUCCESS );
}
