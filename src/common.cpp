#include <boost/core/demangle.hpp>
#include "common.hpp"



std::string 
common::__printClassName( const std::string &&obj_name )
{
   /** user must delete this, make string then delete **/
   return( boost::core::demangle( obj_name.c_str() ) );
}


std::string
common::printClassNameFromStr( const std::string &&str )
{
   return( common::__printClassName( std::move( str ) ));
}
