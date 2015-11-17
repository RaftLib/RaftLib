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
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_C_FLAGS} -O0 -g3 -std=c++11")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_C_FLAGS} -O3 -std=c++11")

file(GLOB_RECURSE FIFO_SRC_FILES ${PROJECT_SOURCE_DIR}/fifo/*.cpp)

add_library(raft allocate.cpp map.cpp graphtools.cpp port.cpp 
            portexception.cpp schedule.cpp simpleschedule.cpp
            stdalloc.cpp portiterator.cpp dynalloc.cpp
            roundrobin.cpp kernel.cpp mapbase.cpp submap.cpp 
            globalmap.cpp systemsignalhandler.cpp poolschedule.cpp 
            kernelcontainer.cpp common.cpp basicparallel.cpp 
            noparallel.cpp affinity.cpp portmap_t.cpp port_info.cpp
            parallelk.cpp ${FIFO_SRC_FILES} 
            fifo/getrandom.c pthreadwrap.c )

set( RAFTDIR ${CMAKE_INSTALL_PREFIX}/include/raft_dir ) 

install( DIRECTORY DESTINATION ${RAFTDIR} )
set( TEMPLATES foreach.tcc join.tcc lambdak.tcc readeach.tcc 
               reduce.tcc split.tcc writeeach.tcc
               math/dct.tcc math/filter.tcc math/raftmath.tcc 
               math/raftmath.tcc math/stat.tcc math/streamingstat.tcc
               packages/chunkiterator.tcc
packages/fastaread.tcc
packages/fileio.tcc
packages/parse.tcc
packages/print.tcc
packages/search.tcc
fifo/SystemClock.tcc
fifo/arrivalratesampletype.tcc
fifo/autorelease.tcc
fifo/bufferdata.tcc
fifo/database.tcc
fifo/datamanager.tcc
fifo/departureratesampletype.tcc
fifo/element.tcc
fifo/fifoabstract.tcc
fifo/meansampletype.tcc
fifo/randomstring.tcc
fifo/ratesampletype.tcc
fifo/ringbuffer.tcc
fifo/ringbufferbase.tcc
fifo/ringbufferheap.tcc
fifo/ringbufferinfinite.tcc
fifo/sample.tcc
fifo/sampletype.tcc

               )

foreach( TEMPL ${TEMPLATES} )
install( FILES ${TEMPL} DESTINATION ${RAFTDIR} )
endforeach( TEMPL ${TEMPLATES} )

set( HEADERS affinity.hpp allocate.hpp basicparallel.hpp
             common.hpp dynalloc.hpp graphtools.hpp
             kernel.hpp kernelcontainer.hpp kernelpreempt.hpp
             leastusedfirst.hpp map.hpp mapbase.hpp noparallel.hpp
             optdef.hpp parallelk.hpp partition_old.hpp poolschedule.hpp
             port.hpp port_info.hpp port_info_types.hpp portbase.hpp
             portexception.hpp portiterator.hpp portmap_t.hpp
             porttemplate.hpp rafttypes.hpp roundrobin.hpp
             sched_cmd_t.hpp schedule.hpp scheduleconst.hpp
             simpleschedule.hpp splitmethod.hpp stdalloc.hpp
             submap.hpp systemsignal.hpp systemsignalhandler.hpp
             typecheck.hpp
fifo/Clock.hpp
fifo/autoreleasebase.hpp
fifo/blocked.hpp
fifo/fifo.hpp
fifo/pointer.hpp
fifo/procwait.hpp
fifo/resolution.hpp
fifo/ringbuffertypes.hpp
fifo/shm.hpp
fifo/signal.hpp
fifo/signaldata.hpp
fifo/signalvars.hpp
fifo/systeminfo.hpp
fifo/systemsignalfifo.hpp
fifo/units.hpp
fifo/getrandom.h 
             )

foreach( HEADER ${HEADERS} )
install( FILES ${HEADER} DESTINATION ${RAFTDIR} )
endforeach( HEADER ${HEADERS} )




install( FILES raft DESTINATION ${RAFTDIR} )
install( FILES raftio DESTINATION ${RAFTDIR} )
install( FILES raftmath DESTINATION ${RAFTDIR} )
install( FILES raftrandom DESTINATION ${RAFTDIR} )
install( FILES raftstat DESTINATION ${RAFTDIR} )
install( FILES raftutility DESTINATION ${RAFTDIR} )


install( TARGETS raft
         ARCHIVE DESTINATION ${CMAKE_INSTALL_PREFIX}/lib )
