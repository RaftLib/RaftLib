list( APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake )
set( CMAKE_INCLUDE_CURRENT_DIR ON )

find_package( Threads )
###
## c/c++ std
###
include( CheckSTD )
#
find_package( LIBRT )
find_package( NUMA  )


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
## compiles doesn't run 
#    allocate_s_string
    allpush
    ap
    bracketsTest
## compiles doesn't run 
#    chain
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
## runs but fails
#    fixedMatchTest
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
#    parallelchain
#    parallel
## compiles doesn't run 
#    partitionTest
## compiles doesn't run 
#    peek
## runs...seems to live lock
#    peekrange
    poolThreadRaftManip
    portTypeException
    prefetchEffective
#    sequential
    splitchain
    splitop
## runs, test fails 
#    splitchainRetStruct
    split
#    splitKsetCont
#    splitKsetJoin
#    staticContJoin
#    staticContJoinRetStruct
#    staticJoin
#    staticJoinRetStruct
#    staticLongSplitChain
## runs, test fails
#    staticLongSplitChainRetStruct
#    staticSplitChainJoin
#    staticSplitChainJoinRetStruct
#    staticSplitJoin
#    staticSplitJoinRetStruct
    stdlibsupport
#    streamManipFourA
#    streamManipFourB
#    streamManipFourC
#    streamManipFour
## runs, test fails
#    streamManipOne
#    streamManipThreeA
#    streamManipThree
## runs, test fails
#    streamManipTwo
#    streamManipZeroA
## runs, test fails
#    streamManipZeroB
## runs, test fails
#    streamManipZeroC
## runs, test fails
#    streamManipZeroD
## runs, test fails
#    streamManipZeroE
#    streamManipZeroF
#    streamManipZeroG
#    testraft10
## compiles doesn't run
#twiceAdd
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
