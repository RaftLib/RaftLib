##
# just check for all of boost, most people install boost-dev-all
##
find_package( Boost 1.59 )
if( Boost_FOUND )
include_directories( ${Boost_INCLUDE_DIRS} )
else( Boost_FOUND )
set( BOOSTDIR ${CMAKE_SOURCE_DIR}/boost-dep )
set( BOOST_MODULES config core random )

foreach( BMODULE ${BOOST_MODULES} )
 message( INFO " Initializing sub-module boost-${BMODULE} from git repo!" )
 execute_process( COMMAND git submodule init ${BOOSTDIR}/${BMODULE}
                  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}) 
 execute_process( COMMAND git submodule update ${BOOSTDIR}/${BMODULE} 
                  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
 include_directories( ${BOOSTDIR}/${BMODULE}/include )
endforeach( BMODULE ${BOOST_MODULES} )
endif( Boost_FOUND )

