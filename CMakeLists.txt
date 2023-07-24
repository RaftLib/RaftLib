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

##
# Build with -fPIC
##
mark_as_advanced( BUILD_FPIC )
set( BUILD_FPIC true CACHE BOOL "Generate position-independent code (-fPIC)" )
if( BUILD_FPIC )
    set(BUILD_FPIC_FLAG "-fPIC")
endif( BUILD_FPIC )


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

##
# setup pkg-config files - for linux/unix/os x
##
if( NOT WIN32 )
##
# make and setup pkg-config
##
mark_as_advanced( PKG_CONFIG_PATHWAY )
set( PKG_CONFIG_PATH "" CACHE STRING "Set the pkg-config path, othwerwise will figure out" )
if( NOT PKG_CONFIG_PATH )
execute_process( COMMAND  pkg-config --variable pc_path pkg-config 
                 COMMAND  tr ':' '\n' 
                 COMMAND  head -n 1
                 OUTPUT_VARIABLE LOCAL_PKG_CONFIG_PATHWAY )
set( PKG_CONFIG_PATH ${LOCAL_PKG_CONFIG_PATHWAY} )
endif()

string(REPLACE ":" ";" PREFIX_LIST "${CMAKE_SYSTEM_PREFIX_PATH}")
list(FIND PREFIX_LIST ${CMAKE_INSTALL_PREFIX}  _index)

if(${_index} GREATER 0)
    file( MAKE_DIRECTORY ${CMAKE_INSTALL_PREFIX}/pkgconfig ) 
    set( PKG_CONFIG_PATH ${CMAKE_INSTALL_PREFIX}/pkgconfig )
    message( WARNING "You should set ${RED}PKG_CONFIG_PATH=${CMAKE_INSTALL_PREFIX}/pkgconfig/:$PKG_CONFIG_PATH ${ColourReset} when installing to non-standard prefix for pkg-config to work correctly!" )
else()
    message( STATUS "Setting PKG_CONFIG_PATH to: ${PKG_CONFIG_PATH}" )
endif()



configure_file( "raftlib.pc.in" "raftlib.pc" @ONLY )
install( FILES ${CMAKE_CURRENT_BINARY_DIR}/raftlib.pc DESTINATION ${CMAKE_INSTALL_PREFIX}/pkgconfig/ )

endif( NOT WIN32 )                     
