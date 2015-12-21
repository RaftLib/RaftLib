cmake_minimum_required(VERSION 3.4 FATAL_ERROR)
project(RaftLib)
set( version 0.7a )
set( CMAKE_INCLUDE_CURRENT_DIR ON )
list( APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake )


include_directories ( ${CMAKE_SOURCE_DIR}/raftinc )
include_directories ( ${CMAKE_SOURCE_DIR} )


add_subdirectory( src )
add_subdirectory( raftinc )

##
# we need boost to demangle names, might use
# for random numbers too
##
include( CheckBoostDep )
##
# c std
##
include( CheckSTD )

##
# Set up unit tests
##
add_subdirectory( testsuite )
enable_testing()
set( TESTAPPS allocate portTypeException dynallocate stdlibsupport split 
     join lambdatest allocate_s foreach peekrange 
     partitionTest iteratort parallel peek )
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
     raftstat )
foreach( HFILE ${MAINHEADERS} )
 install( FILES ${CMAKE_SOURCE_DIR}/${HFILE}  DESTINATION ${CMAKE_INSTALL_PREFIX}/include )
endforeach( HFILE ${MAINHEADERS} )
