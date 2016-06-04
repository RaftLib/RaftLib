##
# check out other repos we need
##
set( DEPDIR ${CMAKE_SOURCE_DIR}/git-dep )
##
# LIST MODULES HERE
##
set( GIT_MODULES cmdargs shm )
##
# NOW CHECK THEM OUT 
##
foreach( GMOD ${GIT_MODULES} )
 message( INFO " Initializing sub-module ${DEPDIR}/${GMOD} from git repo!" )
 execute_process( COMMAND git submodule init ${DEPDIR}/${GMOD}
                  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}) 
 execute_process( COMMAND git submodule update ${DEPDIR}/${GMOD} 
                  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
 ##
 # build execs if needed
 ##
 if( EXISTS ${DEPDIR}/${GMOD}/CMakeLists.txt )
    add_subdirectory( ${DEPDIR}/${GMOD} )
 endif( EXISTS ${DEPDIR}/${GMOD}/CMakeLists.txt )
 ##
 # assume we have an include dir in the sub-module
 ##
 if( EXISTS ${DEPDIR}/${GMOD}/include )
    include_directories( ${DEPDIR}/${GMOD}/include )
 endif( EXISTS ${DEPDIR}/${GMOD}/include )
endforeach( GMOD ${GIT_MODULES} )
