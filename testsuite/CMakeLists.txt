find_package( GSL COMPONENTS gsl gslcblas )
find_package( Threads )

set( TESTAPPS allocate portTypeException dynallocate stdlibsupport split 
     join lambdatest allocate_s foreach peekrange gamma uniform 
     gaussian exponential partitionTest iteratort sequential 
     parallel peek )

##
# start check for RT libs
# var CMAKE_RT_LIBS will default to "" on non-unix platforms
##
if( NOT CMAKE_HOST_UNIX )
    set( CMAKE_RT_LIBS "" )
else( NOT CMAKE_HOST_UNIX )
find_library( RT_LIBRARY
              NAMES rt
              PATHS
              ${CMAKE_LIBRARY_PATH}
              $ENV{RT_PATH}/lib
              /usr/lib
              /usr/local/lib
              /opt/local/lib )
if( RT_LIBRARY )
    set( CMAKE_RT_LIBS ${RT_LIBRARY} )
else()
    set( CMAKE_RT_LIBS "" )
endif()
mark_as_advanced( RT_LIBRARY CMAKE_RT_LIBS )
endif( NOT CMAKE_HOST_UNIX )
## end check for RT libs

include_directories(  ${GSL_INCLUDE_DIRS} )

foreach( APP ${TESTAPPS} )
 add_executable( ${APP} "${APP}.cpp" )
 target_link_libraries( ${APP} raft  ${GSL_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} ${CMAKE_RT_LIBS})
endforeach( APP ${TESTAPPS} )

##
# test cases that need other libs 
##
#add_executable( testraft10 testraft10.cpp )
#target_link_libraries( testraft10 raft ${OpenCV_LIBS} )
