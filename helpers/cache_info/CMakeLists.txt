project(CacheInfo)

add_executable(cache_info cacheinfo.cpp)

# in order to find the executable in the helper CMakeLists.txt we
# have to set the binary output path to something fixed. Since multi-configuration
# generators like VS append the configuration type to this path by default, we override
# binary paths for all possible configuration types explicitly. 
set_target_properties(cache_info
                      PROPERTIES
                      RUNTIME_OUTPUT_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
                      RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
                      RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
                      RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
                      RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")