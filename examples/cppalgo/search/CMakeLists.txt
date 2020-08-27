list( APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake )

find_package( Threads )

set( APP bmh )

add_executable( ${APP} "${APP}.cpp" )


target_link_libraries( ${APP} 
                       raft                       
                       demangle
                       affinity
                       ${CMAKE_THREAD_LIBS_INIT} 
                       ${CMAKE_QTHREAD_LIBS}
                       )

file( COPY alice.txt
      DESTINATION ${CMAKE_CURRENT_BINARY_DIR} )
