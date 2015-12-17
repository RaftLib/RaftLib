cmake_minimum_required(VERSION 3.4 FATAL_ERROR)
project(RaftLib)
set( version 0.7a )
set( CMAKE_INCLUDE_CURRENT_DIR ON )

include_directories ( ${CMAKE_SOURCE_DIR}/raftinc )
include_directories ( ${CMAKE_SOURCE_DIR} )
##
# we need boost to demangle names, might use
# for random numbers too
##
find_package( Boost 1.59 )
if( Boost_FOUND )
include_directories( ${Boost_INCLUDE_DIRS} )
else( Boost_FOUND )
set( BOOSTDIR ${CMAKE_SOURCE_DIR}/boost-dep )
set( BOOST_MODULES config core )
foreach( BMODULE ${BOOST_MODULES} )
 message( INFO " Initializing sub-module boost-${BMODULE} from git repo..." )
 message( INFO " at location ${BOOSTDIR}/${BMODULE}" )
 execute_process( COMMAND git submodule init ${BOOSTDIR}/${BMODULE}
                  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR} )
 execute_process( COMMAND git submodule update ${BOOSTDIR}/${BMODULE} 
                  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR} )
 include_directories( ${BOOSTDIR}/${BMODULE}/include )
endforeach( BMODULE ${BOOST_MODULES} )
endif( Boost_FOUND )
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

#option( BUILD_RANDOM_NUMBER_GEN  "Build the random number generation support" OFF)

#add_custom_command( OUTPUT ${CMAKE_SOURCE_DIR}/dependencies/mathlib
#                    COMMAND perl

if( UNIX )
##
# Check for CXX11 or greater
##
include( CheckCXXCompilerFlag )
check_cxx_compiler_flag( "-std=c++11" COMPILER_SUPPORTS_CXX11 )
if( COMPILER_SUPPORTS_CXX11 )
 set( CMAKE_CXX_STANDARD 11 )
else()
 message( FATAL_ERROR "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a newer compiler" )
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



add_definitions( -DLIBCOMPILE=1  -DRDTSCP=1 )

#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0 -g3")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -g3")

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
