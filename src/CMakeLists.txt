
set(CPP_SRC_FILES 
    allocate.cpp
    basicparallel.cpp
    blocked.cpp
    common.cpp
    dynalloc.cpp
    fifo.cpp
    graphtools.cpp
    kernel.cpp
    kernel_all.cpp
    kernelexception.cpp
    kernel_pair_t.cpp
    kernel_wrapper.cpp
    kpair.cpp
    leastusedfirst.cpp
    makedot.cpp
    mapbase.cpp
    map.cpp
    mapexception.cpp
    noparallel.cpp
    parallelk.cpp
    partition_basic.cpp
    partition_dummy.cpp
    partition_scotch.cpp
    pointer.cpp
    poolschedule.cpp
    port.cpp
    port_info.cpp
    portexception.cpp
    portiterator.cpp
    porttemplate.cpp
    raftexception.cpp
    roundrobin.cpp
    schedule.cpp
    signal.cpp
    signaldata.cpp
    simpleschedule.cpp
    stdalloc.cpp
    submap.cpp
    systemsignalhandler.cpp
)

add_library( raft ${CPP_SRC_FILES} )
target_compile_definitions(raft PRIVATE -DLIBCOMPILE=1 -DRDTSCP=1 )
target_include_directories(raft 
    PUBLIC 
    ${PROJECT_SOURCE_DIR}/raftinc 
    ${PROJECT_SOURCE_DIR}
    ${CMAKE_SCOTCH_INCS}
    ${CMAKE_QTHREAD_INCS}
)

target_link_libraries( 
                       raft 
                       ${CMAKE_SCOTCH_LIBS}
                       ${CMAKE_QTHREAD_LIBS} 
                       demangle
                       affinity
                     )


# Enable warnings if using clang or gcc.
if ( "${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang" 
  OR "${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU" )

target_compile_options( raft
                        PUBLIC
                        ${BUILD_FPIC_FLAG} "-W" "-Wall" "-Wextra" "-Wpedantic" )

endif()

install( TARGETS raft
         ARCHIVE DESTINATION ${CMAKE_INSTALL_PREFIX}/lib )
