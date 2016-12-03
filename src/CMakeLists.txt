set( CMAKE_INCLUDE_CURRENT_DIR ON )
list( APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake )


add_definitions( -DLIBCOMPILE=1 -DRDTSCP=1 )

include_directories( ${CMAKE_SOURCE_DIR}/raftinc 
                     ${CMAKE_SCOTCH_INCS}
                     ${CMAKE_QTHREAD_INCS})
##
# we need boost to demangle names, might use
# for random numbers too
##
include( CheckBoostDep )
##
# c std
##
include( CheckSTD )


file( GLOB_RECURSE CPP_SRC_FILES *.cpp )
file( GLOB_RECURSE C_SRC_FILES   *.c   )


add_library( raft 
             ${CPP_SRC_FILES} 
             ${C_SRC_FILES} )

target_link_libraries( 
                       raft
                       PUBLIC
                       ${CMAKE_SCOTCH_LIBS}
                       ${CMAKE_QTHREAD_LIBS} 
                       shm
                     )


# Enable warnings if using clang or gcc.
if ( "${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang" 
  OR "${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU" )

target_compile_options( raft
                        PUBLIC
                        "-W" "-Wall" "-Wextra" "-Wpedantic" )

endif()

install( TARGETS raft
         ARCHIVE DESTINATION ${CMAKE_INSTALL_PREFIX}/lib )
