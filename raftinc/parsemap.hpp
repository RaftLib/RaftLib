/**
 * parsemap.hpp - 
 * @author: Jonathan Beard
 * @version: Sun Mar 19 05:04:05 2017
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
#ifndef _PARSEMAP_HPP_
#define _PARSEMAP_HPP_  1
#include <stack>
#include <memory>
#include <cstdint>
#include <vector>
#include "parsedefs.hpp"
#include "submap.hpp"

namespace raft
{
/** just pre-declaration **/
class kernel;

class parsemap : public submap
{
    using group_t       = std::vector< raft::kernel* >;
public:
    
    using group_ptr_t   = std::unique_ptr< group_t >;
    
    parsemap();
    virtual ~parsemap();

    void push_state( raft::parse::state * const state );
   
   /** 
    * parse_link - this one is meant to be used
    * by the parser for template overloads. The
    * state struct is defined in parsedefs.hpp
    *
    * @param   a - raft::kernel*, src kernel
    * @param   b - raft::kernel*, dst kernel
    * @param   s - make null if no state, else pass a state struct
    */
    void parse_link( raft::kernel *src, 
                     raft::kernel *dst,
                     const raft::parse::state * const s );

    void start_group();

    void add_to_group( raft::kernel * const k );
   
    std::size_t  get_group_size();
    /**
     * pop_group
     */
    group_ptr_t pop_group();

private:
    /**
     * pop_state - returns the current state from the
     * top of the stack. If no state is available, then 
     * it returns nullptr. This should be called till 
     * no more state is available...users can string 
     * along lots of state and one will be added for each
     * @return raft::parse::state* - nullptr if no state, otherwise valid object
     */
    raft::parse::state* pop_state();
    /**
     * this stack must be empty after each parse, 
     * as in "a >> state >> state >> b" the state
     * must be shoved in between a->b so that it 
     * refers to the right link.
     */
    std::stack< raft::parse::state* > state_stack;
    /**
     * The "parse_head" keeps track of the last kernels
     * added. The last kernels to be added are going to
     * be the ones we want to operate on. The observation
     * that there's no way to create a chain growing towards
     * the terminus of the directed graph.
     */
    std::vector< group_ptr_t >       parse_head;    
};

}
#endif /* END _PARSEMAP_HPP_ */
