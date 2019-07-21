#include <iostream>
#include <cstdlib>
#include <cstdint>
#include <cassert>
#include <cstddef>
#include "alloc_traits.tcc"
#include "defs.hpp"
#include "foodef.tcc"


int
main( int argc, char **argv )
{   
   UNUSED( argc );
   if( fits_in_cache_line< int[ 2 ] >::value != true )
   {
       std::cerr << "test: " << argv[ 0 ] << " failed\n";
       exit( EXIT_FAILURE );
   }
   /** should be false, gotta disambiguate classes from arr/fund **/
   if( inline_class_alloc< int[ 2 ] >::value != false )
   {
       std::cerr << "test: " << argv[ 0 ] << " failed\n";
       exit( EXIT_FAILURE );
   }
   /** this one should be true **/
   if( inline_nonclass_alloc< int[ 2 ] >::value != true )
   {
       std::cerr << "test: " << argv[ 0 ] << " failed\n";
       exit( EXIT_FAILURE );
   }

   /** should be fundamental type, and be true **/
   if( inline_nonclass_alloc< int >::value != true )
   {
       std::cerr << "test: " << argv[ 0 ] << " failed\n";
       exit( EXIT_FAILURE );
   }

   /** should be too big, ret false **/
   if( inline_nonclass_alloc< int[ 128 ] >::value != false )
   {
       std::cerr << "test: " << argv[ 0 ] << " failed\n";
       exit( EXIT_FAILURE );
   }

   /** class type, should be false too **/
   if( inline_nonclass_alloc< foo< 2 > >::value != false )
   {
       std::cerr << "test: " << argv[ 0 ] << " failed\n";
       exit( EXIT_FAILURE );
   }

   return( EXIT_SUCCESS );
}
