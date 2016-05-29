#include <iostream>
#include <cstdlib>
#include <cstdint>
#include <cstddef>
#include <array>
#include "alloc_traits.tcc"
#include "defs.hpp"


template < std::size_t N > class foo
{
public:
   foo( const int a ) : A( a ){}

private:
   const int A;
   char pad[ N ];
};

int
main()
{
    if( fits_in_cache_line< int[ 32 ] >::value != false )
    {
        return( EXIT_FAILURE );
    }
    /** true cases **/
    if( ext_alloc< int[32] >::value != true )
    {
        return( EXIT_FAILURE );
    }
    if( ext_alloc< foo< 100 > >::value != true )
    {
        return( EXIT_FAILURE );
    }
    std::array< int, 100 > f;
    if( ext_alloc< decltype( f ) >::value != true )
    {
        return( EXIT_FAILURE );
    }
    UNUSED( f );
    /** false cases **/
    if( ext_alloc< float >::value != false )
    {
        return( EXIT_FAILURE );
    }
    if( ext_alloc< int >::value != false )
    {
        return( EXIT_FAILURE );
    }
    if( ext_alloc< int[16] >::value != false )
    {
        return( EXIT_FAILURE );
    }
    if( ext_alloc< foo< 60 > >::value != false )
    {
        return( EXIT_FAILURE );
    }
    return( EXIT_SUCCESS );
}
