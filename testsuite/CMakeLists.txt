find_package( GSL COMPONENTS gsl gslcblas )

set( TESTAPPS allocate portTypeException dynallocate stdlibsupport split 
     join lambdatest allocate_s foreach peekrange gamma uniform 
     gaussian exponential partitionTest iteratort sequential 
     parallel peek )

include_directories(  ${GSL_INCLUDE_DIRS} )

foreach( APP ${TESTAPPS} )
 add_executable( ${APP} "${APP}.cpp" )
 target_link_libraries( ${APP} raft  ${GSL_LIBRARIES} )
endforeach( APP ${TESTAPPS} )

##
# test cases that need other libs 
##
#add_executable( testraft10 testraft10.cpp )
#target_link_libraries( testraft10 raft ${OpenCV_LIBS} )
