##
# start check for RT libs
# var CMAKE_RT_LIBS will default to "" on non-unix platforms
##
if( NOT CMAKE_HOST_UNIX OR WIN32 )
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
    set( CMAKE_RT_LINK "-l${RT_LIBRARY}" )
else()
    set( CMAKE_RT_LIBS "" )
endif()
mark_as_advanced( RT_LIBRARY CMAKE_RT_LIBS )
endif( NOT CMAKE_HOST_UNIX )
## end check for RT libs
