cmake_minimum_required(VERSION 3.4 FATAL_ERROR)
project(RaftLib)
set( version 0.7a )
set( CMAKE_INCLUDE_CURRENT_DIR ON )
list( APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake )


include_directories ( ${CMAKE_SOURCE_DIR}/raftinc )
include_directories ( ${CMAKE_SOURCE_DIR} )

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



add_definitions( -DLIBCOMPILE=1  -DRDTSCP=1 )

file(GLOB_RECURSE CPP_SRC_FILES ${CMAKE_SOURCE_DIR}/src/*.cpp)
file(GLOB_RECURSE C_SRC_FILES ${CMAKE_SOURCE_DIR}/src/*.c)

add_library(raft ${CPP_SRC_FILES} ${C_SRC_FILES} )  

##
# we could just copy the dir over, but
# this way takes care of permissions too
##
set( RAFTDIR_NAME "raftinc" )
set( RAFTDIR ${CMAKE_INSTALL_PREFIX}/include/${RAFTDIR_NAME} ) 



##
# create raftinc dir in includes
##
install( DIRECTORY DESTINATION ${RAFTDIR} )

file(GLOB_RECURSE TEMPLATES ${CMAKE_SOURCE_DIR}/${RAFTDIR_NAME}/*.tcc)
file(GLOB_RECURSE CPP_HEADERS ${CMAKE_SOURCE_DIR}/${RAFTDIR_NAME}/*.hpp)
file(GLOB_RECURSE C_HEADERS ${CMAKE_SOURCE_DIR}/${RAFTDIR_NAME}/*.h)

set( INST_INC ${TEMPLATES} ${CPP_HEADERS} ${C_HEADERS} )

foreach( INST_FILE ${INST_INC} )
 install( FILES ${INST_FILE} DESTINATION ${RAFTDIR} )
endforeach( INST_FILE ${INST_INC} )


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

install( TARGETS raft
         ARCHIVE DESTINATION ${CMAKE_INSTALL_PREFIX}/lib )
