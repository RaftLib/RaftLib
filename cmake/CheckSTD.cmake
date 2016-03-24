if( UNIX )
##
# Check for CXX14 or greater
##
include( CheckCXXCompilerFlag )
check_cxx_compiler_flag( "-std=c++14" COMPILER_SUPPORTS_CXX14 )
if( COMPILER_SUPPORTS_CXX14 )
 set( CMAKE_CXX_STANDARD 14 )
else()
 message( FATAL_ERROR "The compiler ${CMAKE_CXX_COMPILER} has no C++14 support. Please use a newer compiler" )
endif()
##
# Check for c99 or greater
##
include( CheckCCompilerFlag )
check_c_compiler_flag( "-std=c99" COMPILER_SUPPORTS_C99 )
if( COMPILER_SUPPORTS_C99 )
 set( CMAKE_C_STANDARD 99 )
else()
 message( FATAL_ERROR "The compiler ${CMAKE_C_COMPILER} has no c99 support. Please use a newer compiler" )
endif()
set( CXX_STANDARD ON )
set( C_STANDARD ON )
endif( UNIX )
if( MSVC )
    set( CMAKE_CXX_STANDARD 14 )
    set( CMAKE_C_STANDARD 99 )
    set( CXX_STANDARD ON )
    set( C_STANDARD ON )
endif( MSVC )
