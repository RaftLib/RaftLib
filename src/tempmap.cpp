/**
 * tempmap.cpp - The primary purpose of this class 
 * is to provide a temporary container that can be 
 * returned by various utility functions that will 
 * be recognized by the raft::map container. This is 
 * for both linquistic recognition withinthe template 
 * grammar and for convenience when house-keeping 
 * kernel allocations. This class is an extension of
 * the mapbase class that adds some public functions
 * to add kernels to the base class containers. 
 * 
 * @author: Jonathan Beard
 * @version: Sat Jan 21 03:12:23 2017
 * 
 * Copyright 2017 Jonathan Beard
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
#include "tempmap.hpp"

raft::temp_map::temp_map() : MapBase()
{
   /** nothing really to do **/
}

raft::temp_map::~temp_map()
{
   /** nothing really to do **/
}
    
void 
raft::temp_map::addSourceKernel( raft::kernel * const k )
{
    assert( k != nullptr );
    /** 
     * implicit acquire release so no chance of a 
     * data race.
     */
    source_kernels += k;
    all_kernels += k;
}

void 
raft::temp_map::addSinkKernels( raft::kernel * const k )
{
    assert( k != nullptr );
    /** 
     * implicit acquire release so no chance of a 
     * data race.
     */
    dst_kernels += k;
    all_kernels += k;
}
    
void 
raft::temp_map::updateKernels( raft::kernel * const a, 
                               raft::kernel * const b )
{
    UNUSED( a );
    UNUSED( b );
    return;
}
