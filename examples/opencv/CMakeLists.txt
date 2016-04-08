list( APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake )

find_package( Threads )
##
# c/c++ std
##
include( CheckSTD )
##
# for demangle, perhaps others
##
include( CheckBoostDep )

find_package( OpenCV 2 REQUIRED )
include_directories( ${OpenCV_INCLUDE_DIRS} )

find_package( LIBRT )
find_package( Scotch )

set( APP contours )

add_executable( ${APP} "${APP}.cpp" )

target_link_libraries( ${APP} 
                       raft  
                       ${CMAKE_THREAD_LIBS_INIT} 
                       ${CMAKE_SCOTCH_LIBS}
                       ${CMAKE_RT_LIBS} 
                       ${OpenCV_LIBS} )
