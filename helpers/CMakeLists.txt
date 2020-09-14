cmake_minimum_required(VERSION 3.10)
project(Helpers)

set( helpers cache_info)

## cache info

# build
set( HELPER_BUILD_DIR ${CMAKE_CURRENT_BINARY_DIR})
execute_process(COMMAND ${CMAKE_COMMAND} -G ${CMAKE_GENERATOR} -DCMAKE_RUNTIME_OUTPUT_DIRECTORY:PATH=${HELPER_BUILD_DIR}/cache_info ${CMAKE_CURRENT_SOURCE_DIR}/cache_info
                WORKING_DIRECTORY ${HELPER_BUILD_DIR})
execute_process(COMMAND ${CMAKE_COMMAND} --build . 
                WORKING_DIRECTORY ${HELPER_BUILD_DIR})

# run
find_program(CACHE_INFO_EXE 
             NAMES 
             cache_info 
             PATHS 
             ${HELPER_BUILD_DIR}
             ${HELPER_BUILD_DIR}/helpers 
             ${HELPER_BUILD_DIR}/cache_info/)

if( NOT CACHE_INFO_EXE )
    message(SEND_ERROR "Unable to find cache info executable")
endif()


if( NOT L1D_LINE_SIZE ) # see if a user already set it 
execute_process( COMMAND ${CACHE_INFO_EXE}
                 OUTPUT_VARIABLE L1D_LINE_SIZE)
endif( NOT L1D_LINE_SIZE )                 

# verify & export
if( NOT L1D_LINE_SIZE )
    set( L1D_LINE_SIZE 64 PARENT_SCOPE )
    message( INFO " Couldn't detect cache line size, set to: ${L1D_LINE_SIZE}" )
else( NOT L1D_LINE_SIZE )
    set( L1D_LINE_SIZE ${L1D_LINE_SIZE} PARENT_SCOPE )
    message( INFO " Detected cache line size, set to: ${L1D_LINE_SIZE}" )
endif( NOT L1D_LINE_SIZE )
