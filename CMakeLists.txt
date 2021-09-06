cmake_minimum_required(VERSION 3.4 FATAL_ERROR)
project(RaftLib)
set( version 0.9a )
set( CMAKE_INCLUDE_CURRENT_DIR ON )
list( APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake )

include(Colors)

option( USEQTHREAD "Use QThread threading library" false )
find_package( QThreads )

##
# c std
##
include( CheckSTD )

## 
# helpers
##
add_subdirectory(helpers)

##
# for cache line size
##
add_definitions( "-DL1D_CACHE_LINE_SIZE=${L1D_LINE_SIZE}" ) 

set( OPTIONAL_MODULES "")

mark_as_advanced( STRING_NAMES )
set( STRING_NAMES true CACHE BOOL "Default is to use legacy string-named ports." )
if( STRING_NAMES )
    set( STRNAMES  "-DSTRING_NAMES=1" )
    add_definitions( ${STRNAMES} )
else()
    list( APPEND OPTIONAL_MODULES highwayhash )
endif( STRING_NAMES )


mark_as_advanced( BENCHMARK_MODE )
set( BENCHMARK_MODE false CACHE BOOL "Set flags and variables to synchronize execution" )
if( BENCHMARK_MODE )
    add_definitions( "-DBENCHMARK=1" )
endif( BENCHMARK_MODE )


##
# need this for examples and testcases
## 
find_package( Threads )
##
# load git submodules
##
include( CheckGitDep )


add_subdirectory( src )
add_subdirectory( raftinc )

mark_as_advanced( BUILD_EXAMPLES )
set( BUILD_EXAMPLES false CACHE BOOL "Examples are built if true" )
if( BUILD_EXAMPLES )
    add_subdirectory( examples )
endif( BUILD_EXAMPLES )


mark_as_advanced( BUILD_WOPENCV )
set( BUILD_WOPENCV false CACHE BOOL "Build OpenCV Examples if true" ) 
if( BUILD_WOPENCV )
    add_subdirectory( examples/opencv )
endif( BUILD_WOPENCV )

##
# Set up unit tests
##
mark_as_advanced( BUILD_TESTS )
set( BUILD_TESTS true CACHE BOOL "Tests build target available if true" )
if( BUILD_TESTS )
    enable_testing()
    add_subdirectory( testsuite )
endif( BUILD_TESTS )

##
# install main headers in ${prefix}/include dir
##
set( MAINHEADERS 
     raft 
     raftio 
     raftmath 
     raftrandom
     raftstat
     raftutility )
foreach( HFILE ${MAINHEADERS} )
 install( FILES ${CMAKE_CURRENT_SOURCE_DIR}/${HFILE}  DESTINATION ${CMAKE_INSTALL_PREFIX}/include )
endforeach( HFILE ${MAINHEADERS} )

include( PkgConfigHelper )
