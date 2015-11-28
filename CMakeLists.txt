cmake_minimum_required(VERSION 3.0 FATAL_ERROR)
project(RaftLib)
set( version 0.7a )
set( CMAKE_INCLUDE_CURRENT_DIR ON )

include(${CMAKE_SOURCE_DIR}/cmake/InstallConfig.cmake)


include_directories (   "${CMAKE_SOURCE_DIR}" 
                        "${CMAKE_SOURCE_DIR}/fifo"
                        "${CMAKE_SOURCE_DIR}/math"
                        "${CMAKE_SOURCE_DIR}/random"
                    )
##
# Set up unit tests
##
add_subdirectory( testsuite )
enable_testing()
set( TESTAPPS allocate portTypeException dynallocate stdlibsupport split 
     join lambdatest allocate_s foreach peekrange gamma uniform 
     gaussian exponential partitionTest iteratort sequential 
     parallel peek )
foreach( TEST ${TESTAPPS} )
 add_test( NAME "${TEST}_test" COMMAND ${TEST} )
endforeach( TEST ${TESTAPPS} )

#option( BUILD_RANDOM_NUMBER_GEN  "Build the random number generation support" OFF)

#add_custom_command( OUTPUT ${CMAKE_SOURCE_DIR}/dependencies/mathlib
#                    COMMAND perl

##
# Check for CXX11 or greater
##
include( CheckCXXCompilerFlag )
check_cxx_compiler_flag( "-std=c++11" COMPILER_SUPPORTS_CXX11 )
if( COMPILER_SUPPORTS_CXX11 )
 set( CMAKE_CXX_STANDARD 11 )
else()
 message( FATAL_ERROR "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a newer compiler" )
endif()

##
# Check for c99 or greater
##
include( CheckCCompilerFlag )
check_c_compiler_flag( "-std=c99" COMPILER_SUPPORTS_C99 )
if( COMPILER_SUPPORTS_C99 )
 set( CMAKE_C_STANDARD 99 )
else()
 message( FATAL_ERROR "The compiler ${CMAKE_C_COMPILER} has no c99 support. Please use a newer compiler" )
endif()
add_definitions( -DLIBCOMPILE=1 )


file(GLOB_RECURSE FIFO_SRC_FILES ${PROJECT_SOURCE_DIR}/fifo/*.cpp)
file(GLOB_RECURSE RANDOM_SRC_FILES ${PROJECT_SOURCE_DIR}/random/*.cpp)

add_library(raft allocate.cpp map.cpp graphtools.cpp port.cpp 
            portexception.cpp schedule.cpp simpleschedule.cpp
            stdalloc.cpp portiterator.cpp dynalloc.cpp
            roundrobin.cpp kernel.cpp mapbase.cpp submap.cpp 
            globalmap.cpp systemsignalhandler.cpp poolschedule.cpp 
            kernelcontainer.cpp common.cpp basicparallel.cpp 
            noparallel.cpp affinity.cpp portmap_t.cpp port_info.cpp
            parallelk.cpp ${FIFO_SRC_FILES} ${RANDOM_SRC_FILES}
            fifo/getrandom.c pthreadwrap.c )

set( RAFTDIR_NAME "raft_dir" )
set( RAFTDIR ${CMAKE_INSTALL_PREFIX}/include/${RAFTDIR_NAME} ) 
file( MAKE_DIRECTORY ${CMAKE_SOURCE_DIR}/${RAFTDIR_NAME} )

#set_property( DIRECTORY 
#              ${CMAKE_SOURCE_DIR}/${RAFTDIR_NAME}
#              PROPERTY
#              ADDITIONAL_MAKE_CLEAN_FILES )

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
random/random.tcc
random/randomx.tcc
               )

foreach( TEMPL ${TEMPLATES} )
file( COPY ${TEMPL} DESTINATION ${CMAKE_SOURCE_DIR}/${RAFTDIR_NAME})
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
random/randombase.hpp
             )

foreach( HEADER ${HEADERS} )
install( FILES ${HEADER} DESTINATION ${RAFTDIR} )
file( COPY ${HEADER} DESTINATION ${CMAKE_SOURCE_DIR}/${RAFTDIR_NAME} )
endforeach( HEADER ${HEADERS} )


set( MAINHEADERS 
     raft 
     raftio 
     raftmath 
     raftrandom
     raftstat )
foreach( HFILE ${MAINHEADERS} )
 install( FILES ${HFILE}  DESTINATION ${RAFTDIR} )
endforeach( HFILE ${MAINHEADERS} )

install( TARGETS raft
         ARCHIVE DESTINATION ${CMAKE_INSTALL_PREFIX}/lib )
