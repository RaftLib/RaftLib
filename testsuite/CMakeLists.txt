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



set( TESTAPPS 
    allocate 
    portTypeException 
    dynallocate 
    stdlibsupport 
    split 
     join 
     lambdatest 
     allocate_s 
     foreach 
     peekrange  
     partitionTest 
     parallel 
     peek 
     bracketsTest 
     gamma
     uniform 
     gaussian
     exponential
     sequential
     chain 
     splitchain 
     staticLongSplitChain 
     staticSplitJoin 
     staticSplitChainJoin 
     staticJoin 
     staticContJoin 
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
     splitKsetCont
     ksetContContJoin
     KsetJoin
     splitKsetJoin
     ksettest 
     poolThreadRaftManip 
     poolThreadRaftManip2
     fixedMatchTest 
     splitchainRetStruct 
     staticContJoinRetStruct
     staticJoinRetStruct
     staticLongSplitChainRetStruct
     staticSplitChainJoinRetStruct
     staticSplitJoinRetStruct 
     chainMultiplePorts )


include_directories( ${CMAKE_SCOTCH_INCS} )

foreach( APP ${TESTAPPS} )
 add_executable( ${APP} "${APP}.cpp" )
 target_link_libraries( ${APP} raft  ${CMAKE_THREAD_LIBS_INIT} 
                                     ${CMAKE_SCOTCH_LIBS}
                                     ${CMAKE_RT_LIBS} 
                                     ${CMAKE_QTHREAD_LIBS}
                                     shm
                                     )
endforeach( APP ${TESTAPPS} )

file( COPY alice.txt
      DESTINATION ${CMAKE_CURRENT_BINARY_DIR} )
