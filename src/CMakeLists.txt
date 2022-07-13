
set(CPP_SRC_FILES
    kpair.cpp
)

add_library( raft ${CPP_SRC_FILES} )
target_compile_definitions(raft PRIVATE -DLIBCOMPILE=1 -DRDTSCP=1 )
target_include_directories(raft
    PUBLIC
    ${PROJECT_SOURCE_DIR}/raftinc
    ${PROJECT_SOURCE_DIR}
    ${CMAKE_SCOTCH_INCS}
    ${QTHREAD_INCLUDE}
    ${PROJECT_BINARY_DIR}/raftinc
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
                        "-W" "-Wall" "-Wextra" "-Wpedantic" )

endif()

install( TARGETS raft
         ARCHIVE DESTINATION ${CMAKE_INSTALL_PREFIX}/lib )
