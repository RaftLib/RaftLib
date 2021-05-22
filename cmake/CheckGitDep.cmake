##
# check out other repos we need
##
set( DEPDIR ${CMAKE_CURRENT_SOURCE_DIR}/git-dep )



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
                  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}) 
 execute_process( COMMAND git submodule update ${DEPDIR}/${GMOD} 
                  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
 ##
 # build execs if needed
 ##
 if( EXISTS ${DEPDIR}/${GMOD}/CMakeLists.txt )
    add_subdirectory( ${DEPDIR}/${GMOD} )
    if( EXISTS ${DEPDIR}/${GMOD}/include )
        set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -I${DEPDIR}/${GMOD}/include" )
    else()
        ##
        # this might cause some spurious includes, however, the alternative
        # would be to parse the folder to search for header files, we might
        # have to do this...but, let's see if this gets us the majority
        # of cases we need. 
        ##
        set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -I${DEPDIR}/${GMOD}" )
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
 else()
    ##
    # this might cause some spurious includes, however, the alternative
    # would be to parse the folder to search for header files, we might
    # have to do this...but, let's see if this gets us the majority
    # of cases we need. 
    ##
    include_directories( ${DEPDIR}/${GMOD} )
 endif()
endforeach( GMOD ${GIT_MODULES} )
