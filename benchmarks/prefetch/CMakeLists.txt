list( APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake )

find_package( Threads )
##
# c/c++ std
##
include( CheckSTD )

find_package( LIBRT )

set( APP prefetchBench )

add_executable( ${APP} "${APP}.cpp" )

target_link_libraries( ${APP} 
                       raft  
                       ${CMAKE_THREAD_LIBS_INIT} 
                       ${CMAKE_RT_LIBS} )
