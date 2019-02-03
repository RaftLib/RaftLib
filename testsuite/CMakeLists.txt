list( APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake )
set( CMAKE_INCLUDE_CURRENT_DIR ON )

find_package( Threads )
###
## c/c++ std
###
include( CheckSTD )
#
find_package( LIBRT )
find_package( NUMA )



set( TESTAPPS 
    allocate
    allocatePopExternal
    allocatePopInternal
    allocatePopInternalObject
    allocatePopPush
    allocateRefWrapperOneLHS
    allocateRefWrapperOneRHS
    allocateRefWrapperTwo
    allocate_s
    allocateSendPush
    allocateSendRandomPush
    allocate_s_string
    allpush
    ap
    bracketsTest
    chain
    chainDynamic
    chainMultiplePorts
    chainMultiplePortsDynamic
#    contSplitKsetJoin
    demangle
    disconnectedGraph
    doesntFitInCacheLine
    dynallocate
#    exponential
    fitInCacheLine
    fixedAllocate
    fixedMatchTest
    foreach
#    gamma
#    gaussian
    isExtAlloc
    isExtArray
    isExtClass
    isInlineClass
    isInlineNonClass
    join
#    ksetContContJoin
#    KsetJoin
    ksettest
    lambdatest
    nonTrivialAllocatorPop
    nonTrivialAllocatorPopExternal
    noPortException
    parallelchain
    parallel
    partitionTest
    peek
    peekrange
    poolThreadRaftManip
    portTypeException
    prefetchEffective
#    sequential
    splitchain
    splitchainRetStruct
    split
#    splitKsetCont
#    splitKsetJoin
#    staticContJoin
#    staticContJoinRetStruct
#    staticJoin
#    staticJoinRetStruct
    staticLongSplitChain
    staticLongSplitChainRetStruct
#    staticSplitChainJoin
#    staticSplitChainJoinRetStruct
#    staticSplitJoin
#    staticSplitJoinRetStruct
    stdlibsupport
#    streamManipFourA
#    streamManipFourB
#    streamManipFourC
#    streamManipFour
    streamManipOne
#    streamManipThreeA
#    streamManipThree
    streamManipTwo
    streamManipZeroA
    streamManipZeroB
    streamManipZeroC
    streamManipZeroD
    streamManipZeroE
#    streamManipZeroF
#    streamManipZeroG
#    testraft10
    twiceAdd
    uniform
     )

foreach( APP ${TESTAPPS} )
 add_executable( ${APP} "${APP}.cpp" )
 target_link_libraries( ${APP} raft  ${CMAKE_THREAD_LIBS_INIT} 
                                     ${CMAKE_SCOTCH_LIBS}
                                     shm
                                     ${CMAKE_RT_LIBS} 
                                     ${CMAKE_QTHREAD_LIBS}
                                     ${CMAKE_NUMA_LIBS}
                                     )
 add_test( NAME "${APP}_test" COMMAND ${APP} )
endforeach( APP ${TESTAPPS} )

file( COPY alice.txt
      DESTINATION ${CMAKE_CURRENT_BINARY_DIR} )
