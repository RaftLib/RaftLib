
include(CheckCXXCompilerFlag)

CHECK_CXX_COMPILER_FLAG("-faligned-new" HAS_ALIGNED_NEW)
if (HAS_ALIGNED_NEW)
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -faligned-new")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -faligned-new")
    ##
    # and for the case where no release/debug is added
    # adding to general definitions given it seems the
    # only way to silence the warnings.
    ##
    add_definitions( "-faligned-new" )
endif()

if( UNIX )
##
# Check for CXX14 or greater
##
if( STRING_NAMES )

check_cxx_compiler_flag( "-std=gnu++17" COMPILER_SUPPORTS_CXX17 )

if( COMPILER_SUPPORTS_CXX17 )
 set( CMAKE_CXX_STANDARD 17 )
else()
 message( FATAL_ERROR "The compiler ${CMAKE_CXX_COMPILER} has no C++17 support. Please use a newer compiler" )
endif()

else()

check_cxx_compiler_flag( "-std=gnu++2a" COMPILER_SUPPORTS_CXX20 )

if( COMPILER_SUPPORTS_CXX20 )
 set( CMAKE_CXX_STANDARD 20 )
else()
 message( FATAL_ERROR "The compiler ${CMAKE_CXX_COMPILER} has no C++20 support. Please use a newer compiler" )
endif()

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
