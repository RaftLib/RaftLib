list( APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake )

if( BUILDRANDOM )
 find_package( GSL COMPONENTS gsl gslcblas )
endif( BUILDRANDOM )

find_package( Threads )
##
# c/c++ std
##
include( CheckSTD )

find_package( LIBRT )

set( TESTAPPS allocate portTypeException dynallocate stdlibsupport split 
     join lambdatest allocate_s foreach peekrange  
     partitionTest iteratort parallel peek )

if( BUILDRANDOM )
list( APPEND TESTAPP gamma uniform gaussian exponential sequential ) 
endif( BUILDRANDOM )

include_directories(  ${GSL_INCLUDE_DIRS} )

foreach( APP ${TESTAPPS} )
 add_executable( ${APP} "${APP}.cpp" )
 target_link_libraries( ${APP} raft  ${GSL_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} ${CMAKE_RT_LIBS})
endforeach( APP ${TESTAPPS} )
