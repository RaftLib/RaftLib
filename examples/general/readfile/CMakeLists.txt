list( APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake )

set( APP readtest)

add_executable( ${APP} "${APP}.cpp" )

target_link_libraries( ${APP} 
                       raft                       
                       demangle
                       affinity
                       ${CMAKE_THREAD_LIBS_INIT} 
                       ${CMAKE_QTHREAD_LIBS}
                       )
