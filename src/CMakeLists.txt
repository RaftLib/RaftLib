set( CMAKE_INCLUDE_CURRENT_DIR ON )
list( APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake )

include_directories( ${CMAKE_SOURCE_DIR}/raftinc )
##
# we need boost to demangle names, might use
# for random numbers too
##
include( CheckBoostDep )
##
# c std
##
include( CheckSTD )

add_definitions( -DLIBCOMPILE=1 -DRDTSCP=1 )

file( GLOB_RECURSE CPP_SRC_FILES *.cpp )
file( GLOB_RECURSE C_SRC_FILES   *.c   )

add_library( raft 
             ${CPP_SRC_FILES} 
             ${C_SRC_FILES}   )

install( TARGETS raft
         ARCHIVE DESTINATION ${CMAKE_INSTALL_PREFIX}/lib )
