list( APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake )


##
# we could just copy the dir over, but
# this way takes care of permissions too
##
set( RAFTDIR_NAME "raftinc" )
set( RAFTDIR ${CMAKE_INSTALL_PREFIX}/include/${RAFTDIR_NAME} ) 

##
# create raftinc dir in includes
##
install( DIRECTORY DESTINATION ${RAFTDIR} )

file( GLOB_RECURSE TEMPLATES    *.tcc   )
file( GLOB_RECURSE CPP_HEADERS  *.hpp   )
file( GLOB_RECURSE C_HEADERS    *.h     )

set( INST_INC ${TEMPLATES} ${CPP_HEADERS} ${C_HEADERS} )

foreach( INST_FILE ${INST_INC} )
    install( FILES ${INST_FILE} DESTINATION ${RAFTDIR} )
endforeach( INST_FILE ${INST_INC} )
