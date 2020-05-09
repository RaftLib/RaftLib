##
# SET SPECIFIC NEEDED OPTIONS
# - will be ignored by platforms that don't include these options
# so not a huge deal. 
##
set( DEMANGLE_CUSTOM_NAMESPACE YES ) 
set( DEMANGLE_NAMESPACE "raft" )

set( AFFINITY_CUSTOM_NAMESPACE YES )
set( AFFINITY_NAMESPACE "raft" )

##
# list git repo dependencies here
##
if( WIN32 )
set( GIT_MODULES 
        cmdargs 
        demangle )
else()
##
set( GIT_MODULES 
        affinity 
        shm 
        cmdargs 
        demangle )
endif()

