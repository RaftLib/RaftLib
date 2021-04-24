##
# SET SPECIFIC NEEDED OPTIONS
# - will be ignored by platforms that don't include these options
# so not a huge deal. 
##
set( DEMANGLE_CUSTOM_NAMESPACE TRUE ) 
set( DEMANGLE_NAMESPACE "raft" )

set( AFFINITY_CUSTOM_NAMESPACE TRUE )
set( AFFINITY_NAMESPACE "raft" )

##
# list git repo dependencies here
##
if( WIN32 )
set( GIT_MODULES 
        cmdargs 
        demangle 
        affinity 
        )
else()
##
set( GIT_MODULES 
        shm 
        cmdargs 
        demangle 
        affinity
        ${OPTIONAL_MODULES}
         ) 
endif()

