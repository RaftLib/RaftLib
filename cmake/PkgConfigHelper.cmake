##
# RULE #1, make ${project_name}.pc.in file with project name in lower case
# RULE #2, you can set a custom name if you wish by using the PKG_CONFIG_IN var
# RULE #3, refer to #1
##

string( TOLOWER ${PROJECT_NAME} PROJECT_NAME_LC )
set( PC_FILE_NAME "${PROJECT_NAME_LC}.pc" )

if( EXISTS "${PROJECT_SOURCE_DIR}/${PROJECT_NAME_LC}.pc.in" )
    set( PKG_CONFIG_SOURCE "${PROJECT_SOURCE_DIR}/${PROJECT_NAME_LC}.pc.in" )
    message( STATUS "Setting pkg-config source file as \"${PKG_CONFIG_SOURCE}\"" )
else()

    if( PKG_CONFIG_IN )
        set( PKG_CONFIG_SOURCE ${PKG_CONFIG_IN} )
        message( STATUS "Setting pkg-config source file as \"${PKG_CONFIG_SOURCE}\" based on config directive." )
    else()
        message( WARNING "No pkg-config input file specified, and none provided in source directory" )
    endif()

endif()


if( NOT WIN32 )
##
# make and setup pkg-config
##
mark_as_advanced( PKG_CONFIG_PATHWAY )
set( PKG_CONFIG_PATH "" CACHE STRING "Set the pkg-config path, othwerwise will figure out" )
if( NOT PKG_CONFIG_PATH )
execute_process( COMMAND  pkg-config --variable pc_path pkg-config 
                 COMMAND  tr ':' '\n' 
                 COMMAND  head -n 1
                 OUTPUT_VARIABLE LOCAL_PKG_CONFIG_PATHWAY )
string(REGEX REPLACE "\n$" "" LOCAL_PKG_CONFIG_PATHWAY "${LOCAL_PKG_CONFIG_PATHWAY}")
set( PKG_CONFIG_PATH ${LOCAL_PKG_CONFIG_PATHWAY} )
endif()

string(REPLACE ":" ";" PREFIX_LIST "${CMAKE_SYSTEM_PREFIX_PATH}")
list(FIND PREFIX_LIST ${CMAKE_INSTALL_PREFIX}  _index)

if(${_index} GREATER 0)
    file( MAKE_DIRECTORY ${CMAKE_INSTALL_PREFIX}/pkgconfig ) 
    set( PKG_CONFIG_PATH ${CMAKE_INSTALL_PREFIX}/pkgconfig )
    message( WARNING "You should set PKG_CONFIG_PATH=${CMAKE_INSTALL_PREFIX}/pkgconfig/:$PKG_CONFIG_PATH when installing to non-standard prefix for pkg-config to work correctly!" )
else()
    message( STATUS "Setting PKG_CONFIG_PATH to: ${PKG_CONFIG_PATH}" )
endif()


##
# actual install stuffs go here
##
configure_file( ${PKG_CONFIG_SOURCE} ${PC_FILE_NAME} @ONLY )
install( FILES ${PROJECT_BINARY_DIR}/${PC_FILE_NAME} DESTINATION  ${PKG_CONFIG_PATH} )
endif( NOT WIN32 )
