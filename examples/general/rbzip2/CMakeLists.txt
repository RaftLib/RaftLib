list( APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake )

find_package( BZip2 REQUIRED)

set( APP rbzip2 )

add_executable( ${APP} "${APP}.cpp" )


target_link_libraries( ${APP} 
                       raft                       
                       demangle
                       affinity
                       cmdargs
                       ${BZIP2_LIBRARIES}
                       ${CMAKE_THREAD_LIBS_INIT} 
                       ${CMAKE_QTHREAD_LIBS}
                       )

file( COPY alice.txt
      DESTINATION ${CMAKE_CURRENT_BINARY_DIR} )
