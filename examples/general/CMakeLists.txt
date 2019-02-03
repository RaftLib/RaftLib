#multiply       rbzip2         singlequeue
#pi             readfile       sum

add_subdirectory( pi )
##
# take out if windows since not that many have bzlib installed
##
#if( NOT ${CMAKE_SYSTEM_NAME} MATCHES "Windows" )
#add_subdirectory( rbzip2 )
#endif( NOT ${CMAKE_SYSTEM_NAME} MATCHES "Windows" )

add_subdirectory( multiply )
add_subdirectory( readfile )
add_subdirectory( singlequeue )
add_subdirectory( sum )
