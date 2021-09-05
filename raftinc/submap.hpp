/**
 * submap.hpp - Defines an interface to create sub-mappings
 * which are basically maps that are allowed to have unconnected
 * inputs and/or outputs that will be connected within a main
 * mapping. The only real rule to these "sub-maps" is that the 
 * names of the output ports must be unique.
 *
 * @author: Jonathan Beard
 * @version: Sun Nov 30 06:12:23 2014
 * 
 * Copyright 2014 Jonathan Beard
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
#ifndef _SUBMAP_HPP_
#define _SUBMAP_HPP_  1
#include <map>
#include <string>

#include "defs.hpp"
#include "mapbase.hpp"

namespace raft
{
    class map;

class submap : public MapBase
{
public:
    submap();
    virtual ~submap();

    /**
     * addInputKernel - takes the kernel k as a 
     * parameter and adds all of k's input ports
     * to this submaps' input structure. 
     * NOTE: duplicate ports are not allowed, i.e., 
     * if there are two input kernels who both
     * have a port "x" defined then an error will
     * be thrown.
     * @param k - raft::kernel * const, output kernel
     */
    void addInputKernel( raft::kernel * const k );

    /**
     * addOutputKernel - takes the kernel k as a 
     * parameter and adds all of k's output ports
     * to this submaps' output structure. 
     * NOTE: duplicate ports are not allowed, i.e., 
     * if there are two output kernels who both
     * have a port "x" defined then an error will
     * be thrown.
     * @param k - raft::kernel * const, output kernel
     */
    void addOutputKernel( raft::kernel * const k );

protected:
   friend class map;
   /** essentially source kernels **/
   std::map< raft::port_key_type, 
             raft::kernel* > input;
   /** essentially dest   kernels **/
   std::map< raft::port_key_type,
             raft::kernel* > output;
};

} /** end namespace raft **/
#endif /* END _SUBMAP_HPP_ */
