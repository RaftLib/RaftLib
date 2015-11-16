cmake_minimum_required(VERSION 2.8 FATAL_ERROR)
project(RaftLib)
set( version 0.7a )

include(${CMAKE_SOURCE_DIR}/cmake/InstallConfig.cmake)

include_directories (   "${CMAKE_SOURCE_DIR}" 
                        "${CMAKE_SOURCE_DIR}/fifo"
                        "${CMAKE_SOURCE_DIR}/math"
                        "${CMAKE_SOURCE_DIR}/random"
                    )

add_definitions( -DLIBCOMPILE=1 )

add_library(raft allocate.cpp map.cpp graphtools.cpp port.cpp 
            portexception.cpp schedule.cpp simpleschedule.cpp
            stdalloc.cpp portiterator.cpp dynalloc.cpp
            roundrobin.cpp kernel.cpp mapbase.cpp submap.cpp 
            globalmap.cpp systemsignalhandler.cpp poolschedule.cpp 
            kernelcontainer.cpp common.cpp basicparallel.cpp 
            noparallel.cpp affinity.cpp portmap_t.cpp port_info.cpp
            parallelk.cpp)
