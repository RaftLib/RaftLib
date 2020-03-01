cmake_minimum_required(VERSION 3.4 FATAL_ERROR)
project(RaftLib)
set( version 0.7a )
set( CMAKE_INCLUDE_CURRENT_DIR ON )
list( APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake )

include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-faligned-new" HAS_ALIGNED_NEW)
if (HAS_ALIGNED_NEW)
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -faligned-new")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -faligned-new")
endif()
    
###
# Grab boost for BGL
###
find_package( Boost
                1.67.0 
                REQUIRED 
                COMPONENTS graph )

option( USEQTHREAD "Use QThread threading library" false )
find_package( QThreads )

##
# c std
##
include( CheckSTD )

set( HELPER_DIR ${CMAKE_CURRENT_SOURCE_DIR}/helpers )

##
# helper exec to get the L1 cache size from Linux/OS X/Win
##
message( ERROR "${CMAKE_CXX_COMPILER} ${HELPER_DIR}/cacheinfo.cpp -o ${HELPER_DIR}/cacheinfo" )
execute_process( COMMAND ${CMAKE_CXX_COMPILER} ${HELPER_DIR}/cacheinfo.cpp -o ${HELPER_DIR}/cacheinfo)
execute_process( COMMAND ${HELPER_DIR}/cacheinfo 
                 COMMAND tr -d '\n' 
                 OUTPUT_VARIABLE L1D_LINE_SIZE )
execute_process( COMMAND rm -f ${HELPER_DIR}/cacheinfo )                 

if( NOT L1D_LINE_SIZE )
    set( L1D_LINE_SIZE 64 )
    message( INFO " Couldn't detect cache line size, set to: ${L1D_LINE_SIZE}" )
else( NOT L1D_LINE_SIZE )
    message( INFO " Detected cache line size, set to: ${L1D_LINE_SIZE}" )
endif( NOT L1D_LINE_SIZE )

##
# for cache line size
##
add_definitions( "-DL1D_CACHE_LINE_SIZE=${L1D_LINE_SIZE}" ) 

include( CheckGitDep )

add_subdirectory( src )
add_subdirectory( raftinc )


mark_as_advanced( BUILD_EXAMPLES )
set( BUILD_EXAMPLES false CACHE BOOL "Examples are built if true" )
if( BUILD_EXAMPLES )
add_subdirectory( examples )
endif( BUILD_EXAMPLES )

mark_as_advanced( BUILD_BENCHMARKS )
set( BUILD_BENCHMARKS false CACHE BOOL "Benchmarks are built if true" )
if( BUILD_BENCHMARKS )
add_subdirectory( benchmarks )
endif( BUILD_BENCHMARKS )

mark_as_advanced( BUILD_WOPENCV )
set( BUILD_WOPENCV false CACHE BOOL "Build with opencv example if true" )
if( BUILD_WOPENCV )
    add_subdirectory( examples/opencv )
endif( BUILD_WOPENCV )


mark_as_advanced( BUILD_TESTS )
set( BUILD_TESTS true CACHE BOOL "Tests build target available if true" )
if( BUILD_TESTS )
##
# Set up unit tests
##
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
     raftstat )
foreach( HFILE ${MAINHEADERS} )
 install( FILES ${CMAKE_CURRENT_SOURCE_DIR}/${HFILE}  DESTINATION ${CMAKE_INSTALL_PREFIX}/include )
endforeach( HFILE ${MAINHEADERS} )

