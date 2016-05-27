/**
 * cacheinfo.cpp - 
 * @author: Jonathan Beard
 * @version: Mon Feb 15 06:15:54 2016
 * 
 * Copyright 2016 Jonathan Beard
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <stdint.h>
#include <cstdlib>
#include <iostream>

#if __APPLE__
#include <errno.h>
#include <sys/sysctl.h>

#elif __linux
#include <fstream>
#include <unistd.h>
#elif ( defined _WIN64 || defined _WIN32 )
#include <windows.h>
#include <malloc.h>
#include <stdio.h>
#include <tchar.h>
#endif

#if ( defined _WIN64 || defined _WIN32 )
/**
 * Note: all windows code adapted from example:
 * https://msdn.microsoft.com/en-us/library/ms683194.aspx
 */
typedef BOOL (WINAPI *LPFN_GLPI)(PSYSTEM_LOGICAL_PROCESSOR_INFORMATION, PDWORD);

// Helper function to count set bits in the processor mask.
DWORD CountSetBits(ULONG_PTR bitMask)
{
    DWORD LSHIFT = sizeof(ULONG_PTR)*8 - 1;
    DWORD bitSetCount = 0;
    ULONG_PTR bitTest = (ULONG_PTR)1 << LSHIFT;    
    DWORD i;
    
    for (i = 0; i <= LSHIFT; ++i)
    {
        bitSetCount += ((bitMask & bitTest)?1:0);
        bitTest/=2;
    }

    return bitSetCount;
}
#endif


int 
main( int argc, char **argv )
{
#if __linux
#ifdef _SC_LEVEL1_DCACHE_LINESIZE
    int64_t val = sysconf( _SC_LEVEL1_DCACHE_LINESIZE );
#else
    int64_t val = -1;
#endif    
    if( val == -1 /** failed, try something else **/ )
    {
        std::ifstream ifs( 
            "/sys/devices/system/cpu/cpu0/cache/index0/coherency_line_size" );
        if( ifs.is_open() )
        {
            ifs >> val;
            ifs.close();
        }
    }
    std::cout << ( val == -1 ? 0 : val );
#elif __APPLE__ 
    uint64_t size =  0;
    size_t   len  = sizeof( size );
    
    if( sysctlbyname( "hw.cachelinesize", 
            &size,
            &len,
            NULL,
            0 ) != 0 /** success code **/ )
    {
        /** don't throw error, handle in config util **/
        size = 0;
    }
    std::cout <<  size;
#elif ( defined _WIN64 || defined _WIN32 )
    size_t line_size = 0;
    DWORD buffer_size = 0;
    DWORD i = 0;
    SYSTEM_LOGICAL_PROCESSOR_INFORMATION * buffer = 0;

    GetLogicalProcessorInformation(0, &buffer_size);
    buffer = (SYSTEM_LOGICAL_PROCESSOR_INFORMATION *)malloc(buffer_size);
    GetLogicalProcessorInformation(&buffer[0], &buffer_size);

    for (i = 0; i != buffer_size / sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION); ++i) 
    {
        if( buffer[i].Relationship == RelationCache && buffer[i].Cache.Level == 1) 
        {
            line_size = buffer[i].Cache.LineSize;
            break;
        }
    }
    std::cout <<  line_size;
    free(buffer);
#else
#error "Unknown platform"
#endif
   return( EXIT_SUCCESS );
}
