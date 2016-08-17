cmake_minimum_required(VERSION 3.4 FATAL_ERROR)
project(RaftLib)
set( version 0.7a )
set( CMAKE_INCLUDE_CURRENT_DIR ON )
list( APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake )
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -mtune=native")
##
# check for Scotch, use if there
##
find_package( Scotch )
option( USESCOTCH "Use Scotch partitioning library" )

option( USEQTHREAD "Use QThread threading library" false )
find_package( QThreads )

##
# c std
##
include( CheckSTD )

set( HELPER_DIR ${CMAKE_SOURCE_DIR}/helpers )

##
# helper exec to get the L1 cache size from Linux/OS X/Win
##
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
##
# for tcmalloc
##
#add_definitions( "-fno-builtin-malloc -fno-builtin-calloc -fno-builtin-realloc -fno-builtin-free" )

include_directories ( ${CMAKE_SOURCE_DIR}/raftinc )
include_directories ( ${CMAKE_SOURCE_DIR} )

include( CheckGitDep )

add_subdirectory( src )
add_subdirectory( raftinc )
add_subdirectory( examples )
add_subdirectory( benchmarks )

mark_as_advanced( BUILD_WOPENCV false )
if( BUILD_WOPENCV )
    add_subdirectory( examples/opencv )
endif( BUILD_WOPENCV )

##
# we need boost to demangle names, might use
# for random numbers too
##
include( CheckBoostDep )

##
# Set up unit tests
##
add_subdirectory( testsuite )
enable_testing()
set( TESTAPPS allocate portTypeException dynallocate stdlibsupport split 
     join lambdatest allocate_s foreach peekrange 
     partitionTest parallel peek
     bracketsTest uniform splitchain
     staticLongSplitChain staticSplitJoin 
     staticSplitChainJoin staticJoin staticContJoin
     fitInCacheLine
     isExtClass
     doesntFitInCacheLine
     isInlineClass
     isExtArray
     isExtAlloc 
     isInlineNonClass 
     allocateSendPush 
     allocateSendRandomPush 
     allocatePopPush 
     allocatePopInternal
     allocatePopInternalObject
     allocatePopExternal
     prefetchEffective 
     parallelchain 
     ksettest 
     fixedMatchTest 
     poolThreadRaftManip )
if( BUILDRANDOM )
    list( APPEND TESTAPPS gamma uniform gaussian exponential sequential ) 
endif( BUILDRANDOM )

foreach( TEST ${TESTAPPS} )
    add_test( NAME "${TEST}_test" COMMAND ${TEST} )
endforeach( TEST ${TESTAPPS} )

##
# install main headers in ${prefix}/include dir
##
set( MAINHEADERS 
     raft 
     raftio 
     raftmath 
     raftrandom
     raftmanip
     raftstat )
foreach( HFILE ${MAINHEADERS} )
 install( FILES ${CMAKE_SOURCE_DIR}/${HFILE}  DESTINATION ${CMAKE_INSTALL_PREFIX}/include )
endforeach( HFILE ${MAINHEADERS} )

