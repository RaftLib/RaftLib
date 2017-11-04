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
find_package( NUMA )


set( TESTAPPS 
    allocate 
    allocateRefWrapperOneRHS
    allocateRefWrapperOneLHS
    chain 
    chainMultiplePorts 
    chainDynamic
    chainMultiplePortsDynamic 
    portTypeException 
    dynallocate 
    stdlibsupport 
    lambdatest 
    allocate_s 
    peek 
    bracketsTest 
    peekrange  
    partitionTest 
    foreach 
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
    streamManipZeroA
    split 
    join 
    splitchain 
#     staticLongSplitChain 
#     staticSplitJoin 
#     staticSplitChainJoin 
#     staticJoin 
#     staticContJoin 
#     parallel 
#     parallelchain 
#     splitKsetCont
#     ksetContContJoin
#     KsetJoin
#     splitKsetJoin
#     ksettest 
#     poolThreadRaftManip 
#     fixedMatchTest 
#     splitchainRetStruct 
#     staticContJoinRetStruct
#     staticJoinRetStruct
#     staticLongSplitChainRetStruct
#     staticSplitChainJoinRetStruct
#     staticSplitJoinRetStruct 
      noPortException
      disconnectedGraph
     )


include_directories( ${CMAKE_SCOTCH_INCS} )

foreach( APP ${TESTAPPS} )
 add_executable( ${APP} "${APP}.cpp" )
 target_link_libraries( ${APP} raft  ${CMAKE_THREAD_LIBS_INIT} 
                                     ${CMAKE_SCOTCH_LIBS}
                                     ${CMAKE_RT_LIBS} 
                                     ${CMAKE_QTHREAD_LIBS}
                                     shm
                                     ${CMAKE_NUMA_LIBS}
                                     )
endforeach( APP ${TESTAPPS} )

file( COPY alice.txt
      DESTINATION ${CMAKE_CURRENT_BINARY_DIR} )
