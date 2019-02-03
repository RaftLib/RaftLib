set( CMAKE_INCLUDE_CURRENT_DIR ON )
list( APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake )


include_directories( ${CMAKE_SOURCE_DIR}/raftinc
                     ${CMAKE_SCOTCH_INCS}
                     ${CMAKE_QTHREAD_INCS} )


                     

set( CPP_SRC_FILES 
        affinity.cpp
        allocate.cpp
        basicparallel.cpp
        blocked.cpp
        common.cpp
        demangle.cpp
        dynalloc.cpp
        fifo.cpp
        graphtools.cpp
        kernel_all.cpp
        kernelcontainer.cpp
        kernel.cpp
        kernelexception.cpp
        kernel_pair_t.cpp
        leastusedfirst.cpp
        mapbase.cpp
        map.cpp
        mapexception.cpp
        noparallel.cpp
        parallelk.cpp
        parsemap.cpp
        partition_basic.cpp
        partition_dummy.cpp
        pointer.cpp
        poolschedule.cpp
        port.cpp
        portexception.cpp
        port_info.cpp
        portiterator.cpp
        portmap_t.cpp
        porttemplate.cpp
        raftexception.cpp
        raftmanip.cpp
        roundrobin.cpp
        schedule.cpp
        signal.cpp
        signaldata.cpp
        simpleschedule.cpp
        stdalloc.cpp
        streamparse.cpp
        submap.cpp
        systemsignalhandler.cpp
        tempmap.cpp
)

add_library( raft 
                ${CPP_SRC_FILES} )

target_include_directories(raft 
    PUBLIC 
    ${PROJECT_SOURCE_DIR}/raftinc 
    ${PROJECT_SOURCE_DIR}
    ${CMAKE_QTHREAD_INCS}
)

target_compile_definitions( raft PRIVATE -DLIBCOMPILE=1 -DRDTSCP=1 )

target_link_libraries( 
                       raft 
                       ${CMAKE_SCOTCH_LIBS} 
                       ${CMAKE_QTHREAD_LIBS} 
                     )


# Enable warnings if using clang or gcc.
if ( "${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang" 
  OR "${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU" )

target_compile_options( raft
                        PUBLIC
                        "-W" "-Wall" "-Wextra" "-Wpedantic" )

endif()

install( TARGETS raft
         ARCHIVE DESTINATION ${CMAKE_INSTALL_PREFIX}/lib )
