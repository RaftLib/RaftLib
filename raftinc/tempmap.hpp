/**
 * tempmap.hpp - The primary purpose of this class 
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
#ifndef _TEMPMAP_HPP_
#define _TEMPMAP_HPP_  1
#include <map>
#include <string>

#include "mapbase.hpp"

namespace raft
{
    class map;

class temp_map : public MapBase
{
public:
    temp_map();
    virtual ~temp_map();

    /**
     * viewing the graph as directed graph, we have
     * the following nomenclature of source and sink
     * if laid out from left to right the source kernels
     * would be on the left and the sink kernels would be 
     * on the right, therefore if we get this from a 
     * duplicate to source function call the kernel
     * that we called the duplcate on will have the sink
     * as the calling kernel, and the source as the
     * leftmost point in the graph. The same logic
     * applies to the duplicate to sink.
     */

    /** 
     * addSourceKernel - add the source kernel to this
     * temp_map object.
     * @param raft::kernel * k, should not be null.
     */
    void addSourceKernel( raft::kernel * const k );

    /** 
     * addSinkKernel - add the sink kernel to this
     * temp_map object.
     * @param raft::kernel * k, should not be null.
     */
    void addSinkKernel( raft::kernel * const k );


    /** 
     * updateKernels - override base class version so
     * that we can avoid duplicate adding of these 
     * kernels to the source and sink..since the 
     * temp map's source and sink are well..special.
     */
    virtual void updateKernels( raft::kernel * const a, 
                                raft::kernel * const b );

protected:
    friend class map;
    /** all needed data structures in mapbase **/
};

} /** end namespace raft **/
#endif /* END _TEMPMAP_HPP_ */
