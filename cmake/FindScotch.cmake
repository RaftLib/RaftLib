##
# start check for SCOTCH libs
# var CMAKE_SCOTCH_LIBS will default to "" on non-unix platforms
##
if( USESCOTCH )

find_library( SCOTCH_LIBRARY
              NAMES scotch
              PATHS
              ${CMAKE_LIBRARY_PATH}
              $ENV{SCOTCH_PATH}/lib
              /usr/lib
              /usr/local/lib
              /opt/local/lib 
              /usr/local/lib/scotch 
              /usr/local/scotch 
              /usr/lib/scotch )

find_library( SCOTCH_ERR_LIBRARY
              NAMES scotcherr
              PATHS
              ${CMAKE_LIBRARY_PATH}
              $ENV{SCOTCH_PATH}/lib
              /usr/lib
              /usr/local/lib
              /opt/local/lib 
              /usr/local/lib/scotch 
              /usr/local/scotch 
              /usr/lib/scotch )
              
if( SCOTCH_LIBRARY )
    ##find path
    find_path( SCOTCH_INCLUDE
               NAMES scotch.h
               PATHS
                ${CMAKE_INCLUDE_PATH}
                /usr/include/
                /usr/local/include/
                /opt/include/
                /opt/local/include/
                /usr/local/include/scotch
                /usr/include/scotch
                )
    if( NOT SCOTCH_INCLUDE )
        message( FATAL_ERROR "User selected to use Scotch partitiong library, but not found in path" )
    endif( NOT SCOTCH_INCLUDE )
    set( CMAKE_SCOTCH_LIBS ${SCOTCH_LIBRARY} )
    list( APPEND CMAKE_SCOTCH_LIBS ${SCOTCH_ERR_LIBRARY} )

    set( CMAKE_SCOTCH_INCS ${SCOTCH_INCLUDE} )
    #set compilation to actually compile scotch
    add_definitions( "-DUSE_PARTITION=1" )
    message( INFO " Using Scotch partitioner" ) 
    message( INFO " LIBRARY: ${SCOTCH_LIBRARY}" ) 
    message( INFO " INCLUDE: ${SCOTCH_INCLUDE}" ) 
else( SCOTCH_LIBRARY )
    set( CMAKE_SCOTCH_LIBS "" )
    message( WARNING " Couldn't find Scotch library" )
endif( SCOTCH_LIBRARY  )

mark_as_advanced( SCOTCH_LIBRARY 
                  SCOTCH_ERR_LIBRARY )

else( USESCOTCH )
    set( CMAKE_SCOTCH_LIBS "" )
    set( CMAKE_SCOTCH_INCS "" )
endif( USESCOTCH )

## end check for SCOTCH libs
