list( APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake )


set( APP pisim )

add_executable( ${APP} "${APP}.cpp" )


target_link_libraries( ${APP} 
                       raft                       
                       demangle
                       cmdargs
                       affinity
                       ${CMAKE_THREAD_LIBS_INIT} 
                       ${CMAKE_QTHREAD_LIBS}
                       )
