list( APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake )

find_package( Threads )
##
# c/c++ std
##
include( CheckSTD )

find_package( LIBRT )
##
# check for Scotch, use if there
##
find_package( Scotch )



set( TESTAPPS allocate 
    portTypeException dynallocate stdlibsupport split 
     join lambdatest allocate_s foreach peekrange  
     partitionTest parallel peek 
     bracketsTest uniform chain 
     splitchain staticLongSplitChain 
     staticSplitJoin staticSplitChainJoin 
     staticJoin staticContJoin 
     fitInCacheLine
     isExtClass
     doesntFitInCacheLine
     isInlineClass
     isExtArray
     isExtAlloc 
     isInlineNonClass 
     allocateSendPush 
     allocateSendRandomPush 
     allocatePopPush 
     allocatePopInternal 
     allocatePopInternalObject
     allocatePopExternal 
     prefetchEffective 
     parallelchain 
     ksettest 
     fixedMatchTest 
     splitchainRetStruct 
     staticContJoinRetStruct
     staticJoinRetStruct
     staticLongSplitChainRetStruct
     staticSplitChainJoinRetStruct
     staticSplitJoinRetStruct 
     chainMultiplePorts
     fixedAllocate
     #disconnectedGraph 
     )

if( BUILDRANDOM )
list( APPEND TESTAPP gamma uniform gaussian exponential sequential ) 
endif( BUILDRANDOM )

include_directories( ${CMAKE_SCOTCH_INCS} )

foreach( APP ${TESTAPPS} )
 add_executable( ${APP} "${APP}.cpp" )
 target_link_libraries( ${APP} raft  ${CMAKE_THREAD_LIBS_INIT} 
                                     ${CMAKE_SCOTCH_LIBS}
                                     ${CMAKE_RT_LIBS} 
                                     ${CMAKE_QTHREAD_LIBS}
                                     )
endforeach( APP ${TESTAPPS} )

file( COPY alice.txt
      DESTINATION ${CMAKE_CURRENT_BINARY_DIR} )
