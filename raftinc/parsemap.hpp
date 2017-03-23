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
#include "parsedefs.hpp"
#include "submap.hpp"

namespace raft
{
class parsemap : public submap
{
public:
    parsemap();
    virtual ~parsemap();

    void push_state( raft::parse::state * const state );

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

    std::stack< raft::parse::state* > state_stack;
};

}
#endif /* END _PARSEMAP_HPP_ */
