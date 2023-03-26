#include <iostream>
#include <cstdlib>
#include <cstdint>
#include <cstddef>
#include <array>
#include "alloc_traits.tcc"
#include "defs.hpp"
#include "foodef.tcc"
#include "commontestdefs.hpp"


/**
 * FIXME: this test case setup should rely on multiples
 * of L1D_CACHE_LINE_SIZE vs. raw hard coded numbers..
 * if we move to a 128B or 32B cache line theen these
 * cases will fail or succeed haphazardly. - jcb 1 Sept 2019
 */



int
main()
{
    if( fits_in_cache_line< bigger_than_cache_t  >::value != false )
    {
        return( EXIT_FAILURE );
    }
    /** true cases **/
    if( ext_alloc< bigger_than_cache_t >::value != true )
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
    /** 
     * made this class an external one, which is now int vs. char, 
     * fixing the qantity allocated, eventually we need to fix the
     * hardcoding to something that is dynamic. 
     */
    //if( ext_alloc< foo< 60 > >::value != false )
    /**
     * this should hit internal ctor alloc and ret
     * false.
     */
    if( ext_alloc< foo< 15 > >::value != false )
    {
        return( EXIT_FAILURE );
    }
    return( EXIT_SUCCESS );
}
