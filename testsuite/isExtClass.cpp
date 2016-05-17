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
   char __attribute__((__unused__)) pad[ N ];
};

int
main()
{
   assert( fits_in_cache_line< foo< 4 > >::value );
   /** should be inline class allocate **/
   assert( inline_class_alloc< foo< 4 > >::value );
   
   /** should be fundamental type, and be false **/
   assert( inline_class_alloc< int >::value == false );
   /** should be too big, ret false **/
   assert( inline_class_alloc< foo< 128 > >::value == false );

   /** should be false, not a class **/
   assert( inline_class_alloc< int[ 2 ] >::value == false );
}
