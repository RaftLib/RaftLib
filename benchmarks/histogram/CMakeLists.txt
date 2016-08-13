list( APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake )

find_package( Threads )
##
# check for Scotch, use if there
##
find_package( Scotch )
##
# c/c++ std
##
include( CheckSTD )
##
# for demangle, perhaps others
##
include( CheckBoostDep )
find_package( LIBRT )

set( GITDEP "${CMAKE_SOURCE_DIR}/git-dep" )
##
# grab include directories for git-dep
##
if( EXISTS ${GITDEP} )
##
# get the dirs that are in the git-dep folder
##
file( GLOB DEPFOLDERLIST ${GITDEP}/* )
foreach( DEPFOLDER ${DEPFOLDERLIST} )
    message( STATUS "Checking: ${DEPFOLDER}" )
    if( IS_DIRECTORY ${DEPFOLDER} )
        message( STATUS "Found: ${DEPFOLDER}" )
        include_directories( ${DEPFOLDER}/include )
        link_directories( ${DEPFOLDER}/lib )
    endif( IS_DIRECTORY ${DEPFOLDER} )
endforeach( DEPFOLDER ${DEPFOLDERLIST} )

endif( EXISTS ${GITDEP} )

set( APP sumapp )


include_directories( ${CMAKE_SCOTCH_INCS} )
add_executable( hist hist.c
                     MapReduceScheduler.c )

target_link_libraries( hist
                       ${CMAKE_THREAD_LIBS_INIT} 
                       ${CMAKE_RT_LIBS} )
                       

#add_executable( ${APP} "${APP}.cpp" )
#
#target_link_libraries( ${APP} 
#                       raft
#                       ${CMAKE_SCOTCH_LIBS}
#                       ${CMAKE_RT_LIBS} )
#
