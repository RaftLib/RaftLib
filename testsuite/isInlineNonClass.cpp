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
   char pad[ N ];
};


int
main( int argc, char **argv )
{
   assert( fits_in_cache_line< int[ 2 ] >::value );
   /** should be false, gotta disambiguate classes from arr/fund **/
   assert( inline_class_alloc< int[ 2 ] >::value == false );
   /** this one should be true **/
   assert( inline_nonclass_alloc< int[ 2 ] >::value  );

   /** should be fundamental type, and be true **/
   assert( inline_nonclass_alloc< int >::value  );

   /** should be too big, ret false **/
   assert( inline_nonclass_alloc< int[ 128 ] >::value == false );

   /** class type, should be false too **/
   assert( inline_nonclass_alloc< foo< 2 > >::value == false );
}
