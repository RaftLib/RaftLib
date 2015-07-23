#include "common.hpp"



std::string 
common::__printClassName( const std::string &&obj_name )
{
   enum FailureCods : std::int32_t { InvalidArg = -3, 
                                     InvalidName = -2,
                                     MemAllocationFailure = -1,
                                     DemangleSuccess = 0 };
   std::int32_t status( 0 );
   /** user must delete this, make string then delete **/
   char *str( abi::__cxa_demangle( obj_name.c_str(), 0, 0, &status ) );
   std::string out_str( str );
   std::free( str );
   switch( status )
   {
      case( DemangleSuccess ):
      break; /** one good case **/
      case( InvalidArg ):
      {
         return( "invalid argument provided to cxa_demangle!" );
      }
      break;
      case( InvalidName ):
      {
         return( "invalid name provided to cxa_demangle" );
      }
      break;
      case( MemAllocationFailure ):
      {
         return( "memory allocation error, can't demangle class name" );
      }
      break;
      default:
         assert( false );
   }
   return( out_str );

}


std::string
common::printClassNameFromStr( const std::string &&str )
{
   return( std::move< std::string >( common::__printClassName( std::move( str ) ) ) );
}
