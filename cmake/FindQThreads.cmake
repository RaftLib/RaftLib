##
# start check for QTHREAD libs
# var CMAKE_QTHREAD_LIBS will default to "" on non-unix platforms
##

set( CMAKE_QTHREAD_LIBS "" )
set( CMAKE_QTHREAD_INCS "" )
set( CMAKE_QTHREAD_FLAGS "" )
set( CMAKE_QTHREAD_LDFLAGS "" )
set( QTHREAD_FOUND FALSE )

find_library( QTHREAD_LIBRARY
              NAMES qthread
              PATHS
              ${CMAKE_LIBRARY_PATH}
              $ENV{QTHREAD_LIB}
              $ENV{QTHREAD_PATH}/lib
              /usr/lib
              /usr/local/lib
              /opt/local/lib 
              /usr/local/lib/qthread 
              /usr/local/qthread 
              /usr/lib/qthread )

find_path( QTHREAD_INCLUDE
           NAMES qthread/qthread.hpp
           PATHS
           ${CMAKE_INCLUDE_PATH}
           $ENV{QTHREAD_INC}/
           $ENV{QTHREAD_PATH}/include
           /usr/include/
           /usr/local/include/
           /opt/include/
           /opt/local/include/
           /usr/local/include/qthread
           /usr/include/qthread )


if( QTHREAD_LIBRARY AND QTHREAD_INCLUDE )
    get_filename_component( QTHREAD_LIBRARY ${QTHREAD_LIBRARY} DIRECTORY )
    link_directories( ${QTHREAD_LIBRARY} )
    set( QTHREAD_FOUND TRUE )
    set( CMAKE_QTHREAD_FLAGS  "-DQTHREAD_FOUND" )
    set( CMAKE_QTHREAD_LIBS  "-lqthread" )
    set( CMAKE_QTHREAD_INCS "-I${QTHREAD_INCLUDE}" )
    set( CMAKE_QTHREAD_LDFLAGS "-L${QTHREAD_LIBRARY}" )
    message( STATUS "Qthread threading library available" )
    message( STATUS "LIBRARY: ${QTHREAD_LIBRARY}" ) 
    message( STATUS "INCLUDE: ${QTHREAD_INCLUDE}" ) 
else( QTHREAD_LIBRARY AND QTHREAD_INCLUDE )
    set( QTHREAD_INCLUDE "" )
    message( WARNING "Couldn't find Qthread library" )
endif( QTHREAD_LIBRARY AND QTHREAD_INCLUDE )

mark_as_advanced( QTHREAD_LIBRARY ) 

## end check for QTHREAD libs
