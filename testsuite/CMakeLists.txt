list( APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake )

##
# c/c++ std
##
include( CheckSTD )

if( STRING_NAMES )
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
     parallelchain 
     ksettest
     multiPrint
     fixedMatchTest 
     poolThreadRaftManip
     select_in_t
     splitchainRetStruct 
     staticContJoinRetStruct
     staticJoinRetStruct
     staticLongSplitChainRetStruct
     staticSplitChainJoinRetStruct
     staticSplitJoinRetStruct 
     chainMultiplePorts
     #fixedAllocate
     noPortException
     twiceAdd
     disconnectedGraph 
     allocate_s_string
     allpush
     nonTrivialAllocatorPop
     nonTrivialAllocatorPopExternal
     vectorAlloc
     stringAlloc
     reduction
     )
else()
set( TESTAPPS 
        allocate
        allocatePopExternal 
   )
endif()

if( BUILDRANDOM )
list( APPEND TESTAPP gamma uniform gaussian exponential sequential ) 
endif( BUILDRANDOM )

foreach( APP ${TESTAPPS} )
 add_executable( ${APP} "${APP}.cpp" )
 target_link_libraries( ${APP}  
                                     raft
                                     demangle
                                     affinity
                                     ${CMAKE_THREAD_LIBS_INIT} 
                                     ${CMAKE_QTHREAD_LIBS}
                                     )
 add_test( NAME "${APP}_test" COMMAND ${APP} )
endforeach( APP ${TESTAPPS} )

file( COPY alice.txt
      DESTINATION ${CMAKE_CURRENT_BINARY_DIR} )
