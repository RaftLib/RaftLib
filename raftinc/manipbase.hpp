/**
 * manipbase.hpp - 
 * @author: Jonathan Beard
 * @version: Wed Apr  7 06:40:24 2021
 * 
 * Copyright 2021 Jonathan Beard
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
#ifndef MANIPBASE_HPP
#define MANIPBASE_HPP  1
#include <raft>
#include "port_info.hpp"

namespace raft
{

class manipbase
{
public:
    constexpr manipbase() = default;
    
    /**
     * NOTES: keep in mind that when implementing a 
     * sub-class of these functions that there might 
     * be data fields that must be added to the kernel.hpp
     * or the port_info.hpp files to be able to use them
     * in the first place. Please add those fields to the
     * correct files and ensure that you note which 
     * manip object they map back to with comments. 
     */

    /**
     * invoke - all subclasses must implement this function
     * if they are to be invoked on a kernel. 
     * it is the primary way that manip objects impact the 
     * characteristics of the graph. 
     * @return true if invoke is implemented. 
     */
    virtual bool invoke( raft::kernel &k );
    

    /**
     * invoke - all subclasses must implement this function
     * if they are to be invoked on a graph edge. 
     * it is the primary way that manip objects impact the 
     * characteristics of the graph. 
     * @return true if invoke is implemented. 
     */
    virtual bool invoke( PortInfo &src, PortInfo &dst );

    /**
     * get_next - this function returns the next manip
     * object in the chain. Keep in mind that this list
     * is heterogeneous, being of many derived types, each
     * will use the virtual invoke command on the kernel 
     * to performa its actions. This function returns nullptr
     * if there are no more manip objects. 
     * @param m - valid non-null raft::manip_base (or derived) object.
     * @return - the next manip object in the linked-list, nullptr
     * if there are no more objects. 
     */
    static raft::manipbase* get_next( raft::manip_base *m );

private:
    /**
     * keep a list of manip objects, we don't need to keep the 
     * head b/c it's kept at the kernel. We do, however, need
     * to make sure this list is destructed by the owning kernel. 
     */
    raft::manipbase *next = nullptr;

};

} /** end namespace raft **/ 

#endif /* END MANIPBASE_HPP */
