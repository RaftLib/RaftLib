list( APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake )

find_package( Threads )
##
# check for Scotch, use if there
##
find_package( Scotch )
##
# c/c++ std
##
include( CheckSTD )

find_package( LIBRT )


set( APP bmh )

add_executable( ${APP} "${APP}.cpp" )

include_directories( ${CMAKE_SCOTCH_INCS} )

target_link_libraries( ${APP} 
                       raft                       
                       ${CMAKE_THREAD_LIBS_INIT} 
                       ${CMAKE_SCOTCH_LIBS}
                       ${CMAKE_RT_LIBS}
                       )

file( COPY alice.txt
      DESTINATION ${CMAKE_CURRENT_BINARY_DIR} )
