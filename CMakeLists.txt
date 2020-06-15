cmake_minimum_required(VERSION 3.4 FATAL_ERROR)
project(RaftLib)
set( version 0.9a )
set( CMAKE_INCLUDE_CURRENT_DIR ON )
list( APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake )

include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-faligned-new" HAS_ALIGNED_NEW)
if (HAS_ALIGNED_NEW)
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -faligned-new")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -faligned-new")
endif()
    
    

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


##
# get pkg-config for install - for unix/linux/mac only 
##
if( NOT WIN32 )
mark_as_advanced( PKG_CONFIG_PATHWAY )
set( PKG_CONFIG_PATH "" CACHE STRING "Set the pkg-config path, othwerwise will figure out" )
if( NOT PKG_CONFIG_PATH )
execute_process( COMMAND  pkg-config --variable pc_path pkg-config 
                 COMMAND  tr ':' '\n' 
                 COMMAND  head -n 1
                 OUTPUT_VARIABLE LOCAL_PKG_CONFIG_PATHWAY )
set( PKG_CONFIG_PATH ${LOCAL_PKG_CONFIG_PATHWAY} )
endif()
message( STATUS "Setting PKG_CONFIG_PATH to: ${PKG_CONFIG_PATH}" )
endif( NOT WIN32 )


include( CheckGitDep )

add_subdirectory( src )
add_subdirectory( raftinc )

mark_as_advanced( BUILD_EXAMPLES )
set( BUILD_EXAMPLES false CACHE BOOL "Examples are built if true" )
if( BUILD_EXAMPLES )
    add_subdirectory( examples )
endif( BUILD_EXAMPLES )


mark_as_advanced( BUILD_WOPENCV )
set( BUILD_BENCHMARKS false CACHE BOOL "Build OpenCV Examples if true" ) 
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
     raftstat )
foreach( HFILE ${MAINHEADERS} )
 install( FILES ${CMAKE_CURRENT_SOURCE_DIR}/${HFILE}  DESTINATION ${CMAKE_INSTALL_PREFIX}/include )
endforeach( HFILE ${MAINHEADERS} )

##
# setup pkg-config files - for linux/unix/os x
##
if( NOT WIN32 )

if( NOT ${USEQTHREAD} ) 
install( CODE "file( COPY  ${CMAKE_CURRENT_SOURCE_DIR}/raftlib.pc
                     DESTINATION ${CMAKE_CURRENT_BINARY_DIR} )" )
else()
install( CODE "file( COPY  ${CMAKE_CURRENT_SOURCE_DIR}/raftlib.pc-qthreads
                     DESTINATION ${CMAKE_CURRENT_BINARY_DIR} )" )
install( CODE "file( RENAME ${CMAKE_CURRENT_BINARY_DIR}/raftlib.pc-qthreads ${CMAKE_CURRENT_BINARY_DIR}/raftlib.pc )" )                     
endif( NOT ${USEQTHREAD} )


install( CODE "file( COPY  ${CMAKE_CURRENT_BINARY_DIR}/raftlib.pc
                     DESTINATION ${PKG_CONFIG_PATH} )" )
endif( NOT WIN32 )                     
