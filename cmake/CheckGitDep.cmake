##
# check out other repos we need
##
set( DEPDIR ${CMAKE_SOURCE_DIR}/git-dep )

##
# LIST MODULES HERE
##
include( ${DEPDIR}/gitmodules.cmake )
##
# NOW CHECK THEM OUT 
##
include(ExternalProject)

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
    if( EXISTS ${DEPDIR}/${GMOD}/include )
        set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -I${DEPDIR}/${GMOD}/include" )
    endif()
    ##
    # A bit of a silly hack to pull in flags from sub-modules
    # that we have when we want to inject namespaces into the 
    # sub-modules. 
    ##
    if( EXISTS ${DEPDIR}/${GMOD}/moduleflags.cmake )
        include( ${DEPDIR}/${GMOD}/moduleflags.cmake )
    endif()
 elseif( EXISTS ${DEPDIR}/${GMOD}/autogen.sh )
    ##TODO, need to cleanup in-source build manually...should fix
    message( STATUS "Found automake dir in git-dep, attempting to incorporate..." )
    ExternalProject_Add( ${GMOD}
        SOURCE_DIR ${DEPDIR}/${GMOD}
        CONFIGURE_COMMAND ${DEPDIR}/${GMOD}/autogen.sh && ${DEPDIR}/${GMOD}/configure --prefix=${DEPDIR}/${GMOD}
        TEST_COMMAND make test
        BUILD_COMMAND make clean && make
        INSTALL_COMMAND make install
        BUILD_IN_SOURCE 1 )
 endif( EXISTS ${DEPDIR}/${GMOD}/CMakeLists.txt )
 ##
 # assume we have an include dir in the sub-module
 ##
 if( EXISTS ${DEPDIR}/${GMOD}/include )
    include_directories( ${DEPDIR}/${GMOD}/include )
 endif( EXISTS ${DEPDIR}/${GMOD}/include )
endforeach( GMOD ${GIT_MODULES} )
