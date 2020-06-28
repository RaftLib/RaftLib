##
# start check for NUMA libs
# var CMAKE_NUMA_LIBS will default to "" on non-unix platforms
##
if( CMAKE_HOST_UNIX )
find_library( NUMA_LIBRARY
              NAMES numa
              PATHS
              ${CMAKE_LIBRARY_PATH}
              $ENV{NUMA_PATH}/lib
              /usr/lib
              /usr/local/lib
              /opt/local/lib )
if( NUMA_LIBRARY )
    set( CMAKE_NUMA_LIBS ${NUMA_LIBRARY} )
else( NUMA_LIBRARY )
##
# get machine type
##
    execute_process( COMMAND uname -m COMMAND tr -d '\n' OUTPUT_VARIABLE ARCHITECTURE )
    execute_process( COMMAND ${CMAKE_SOURCE_DIR}/helpers/findnodes.pl
                     COMMAND tr -d '\n' 
                     OUTPUT_VARIABLE HASNUMA )
    message( INFO "Your system has NUMA, variable set to (${HASNUMA})" );                     
    if( HASNUMA EQUAL 0 )
        ## no NUMA
        message( STATUS "no NUMA needed" )
    else()
        ## needs NUMA but we don't have it
        message( FATAL_ERROR "You are compiling on a NUMA system, you must install libnuma" )
    endif()
endif( NUMA_LIBRARY )

endif( CMAKE_HOST_UNIX )
