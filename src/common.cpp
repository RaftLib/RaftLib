#include "common.hpp"
#include "demangle.hpp"


std::string 
common::__printClassName( const std::string &&obj_name )
{
   return( raft::demangle(obj_name.c_str()) );
}


std::string
common::printClassNameFromStr( const std::string &&str )
{
   return( common::__printClassName( std::move( str ) ));
}
