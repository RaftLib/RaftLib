##
# SET SPECIFIC NEEDED OPTIONS 
##
set( CUSTOM_NAMESPACE YES ) 
set( DEMANGLE_NAMESPACE "raft" )
set( AFFINITY_NAMESPACE "raft" )

##
# list git repo dependencies here
##
if( WIN32 )
set( GIT_MODULES 
        cmdargs 
        demangle )
else()
set( GIT_MODULES 
        affinity 
        shm 
        cmdargs 
        demangle )
endif()


##
# TODO ITEMS
##
# 1) export CXXFLAGS here as an option, list them here, action 
# takes place in /cmake/CheckGitDep.cmake file, same for libs
##
