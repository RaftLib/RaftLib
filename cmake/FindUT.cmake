##
# start check for libut
##

set( CMAKE_UT_LIBS "" )
set( CMAKE_UT_INCS "" )
set( CMAKE_UT_FLAGS "" )
set( CMAKE_UT_LDFLAGS "" )
set( UT_FOUND FALSE )

find_library( UT_LIBRARY
              NAMES ut
              PATHS
              ${CMAKE_LIBRARY_PATH}
              $ENV{UT_LIB}
              $ENV{UT_PATH}/lib
              /usr/local/lib )

find_library( UT_BASE_LIBRARY
              NAMES base
              PATHS
              ${CMAKE_LIBRARY_PATH}
              $ENV{UT_LIB}
              $ENV{UT_PATH}/lib
              /usr/local/lib )

find_library( UT_RUNTIME_LIBRARY
              NAMES runtime
              PATHS
              ${CMAKE_LIBRARY_PATH}
              $ENV{UT_LIB}
              $ENV{UT_PATH}/lib
              /usr/local/lib )

find_library( UT_RT_LIBRARY
              NAMES rt++
              PATHS
              ${CMAKE_LIBRARY_PATH}
              $ENV{UT_LIB}
              $ENV{UT_PATH}/lib
              /usr/local/lib )

find_path( UT_INCLUDE
           NAMES ut.h
           PATHS
           ${CMAKE_INCLUDE_PATH}
           $ENV{UT_INC}/
           $ENV{UT_PATH}/include/ut
           /usr/local/include/ut )

if( UT_BASE_LIBRARY AND UT_RUNTIME_LIBRARY AND UT_RT_LIBRARY AND UT_INCLUDE )
    link_libraries( ${UT_RT_LIBRARY} ${UT_RUNTIME_LIBRARY} ${UT_BASE_LIBRARY} )
    set( UT_FOUND TRUE )
    set( UT_DEFINES  "-D_GNU_SOURCE -DTLS_LOCAL_EXEC -DNDEBUG -DUT_FOUND" )
    set( CMAKE_UT_INCS "-I${UT_INCLUDE}" )
    set( CMAKE_UT_FLAGS "${UT_DEFINES}" )
    set( CMAKE_UT_LIBS  "${UT_RT_LIBRARY} ${UT_RUNTIME_LIBRARY} ${UT_BASE_LIBRARY}" )
    set( CMAKE_UT_LDFLAGS "-no-pie" )
    message( STATUS "libut threading library available" )
    message( STATUS "LIBRARY: ${UT_RT_LIBRARY} ${UT_RUNTIME_LIBRARY} ${UT_BASE_LIBRARY}" )
    message( STATUS "INCLUDE: ${UT_INCLUDE}" )
else( UT_BASE_LIBRARY AND UT_RUNTIME_LIBRARY AND UT_RT_LIBRARY AND UT_INCLUDE )
    set( UT_INCLUDE "" )
    message( WARNING "Couldn't find libut library" )
endif( UT_BASE_LIBRARY AND UT_RUNTIME_LIBRARY AND UT_RT_LIBRARY AND UT_INCLUDE )

## end check for libut
