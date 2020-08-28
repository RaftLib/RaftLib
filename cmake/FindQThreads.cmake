##
# start check for QTHREAD libs
# var CMAKE_QTHREAD_LIBS will default to "" on non-unix platforms
##
if( ${USEQTHREAD} )

find_library( QTHREAD_LIBRARY
              NAMES qthread
              PATHS
              ${CMAKE_LIBRARY_PATH}
              $ENV{QTHREAD_PATH}/lib
              /usr/lib
              /usr/local/lib
              /opt/local/lib 
              /usr/local/lib/qthread 
              /usr/local/qthread 
              /usr/lib/qthread )

              
if( QTHREAD_LIBRARY )
    ##find path
    find_path( QTHREAD_INCLUDE
               NAMES qthread.hpp
               PATHS
                ${CMAKE_INCLUDE_PATH}
                /usr/include/
                /usr/local/include/
                /opt/include/
                /opt/local/include/
                /usr/local/include/qthread
                /usr/include/qthread
                )
    if( NOT QTHREAD_INCLUDE )
        message( FATAL_ERROR "User selected to use Qthread partitiong library, but not found in path" )
    endif( NOT QTHREAD_INCLUDE )
    get_filename_component( QTHREAD_LIBRARY ${QTHREAD_LIBRARY} DIRECTORY )
    set( CMAKE_QTHREAD_LDFLAGS "-L${QTHREAD_LIBRARY}" )
    link_directories( ${QTHREAD_LIBRARY} )
    set( CMAKE_QTHREAD_LIBS  "-lqthread" )
    set( CMAKE_QTHREAD_INCS "-I${QTHREAD_INCLUDE}" )
    #set compilation to actually compile qthread
    set( CMAKE_QTHREAD_FLAGS "-DUSEQTHREADS=1" )
    add_definitions( ${CMAKE_QTHREAD_FLAGS} )
    message( STATUS "Using Qthread threading library" ) 
    message( STATUS "LIBRARY: ${QTHREAD_LIBRARY}" ) 
    message( STATUS "INCLUDE: ${QTHREAD_INCLUDE}" ) 
else( QTHREAD_LIBRARY )
    set( CMAKE_QTHREAD_LIBS "" )
    message( WARNING "Couldn't find Qthread library" )
endif( QTHREAD_LIBRARY  )

mark_as_advanced( QTHREAD_LIBRARY ) 

else( ${USEQTHREAD} )
    
    set( CMAKE_QTHREAD_LIBS "" )
    set( CMAKE_QTHREAD_INCS "" )
    set( CMAKE_QTHREAD_FLAGS "" )

endif( ${USEQTHREAD} )

## end check for QTHREAD libs
