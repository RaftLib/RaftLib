/**
 * parsetree.hpp - 
 * @author: Jonathan Beard
 * @version: Mon Nov 11 12:10:48 2019
 * 
 * Copyright 2019 Jonathan Beard
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
#ifndef _PARSETREE_HPP_
#define _PARSETREE_HPP_  1
#include "parsedefs.hpp"
#include "parsetreedefs.hpp"
#include <utility>

namespace raft
{
/** pre-declaration **/
class parsetreedefs;

{
template< class A > struct ptree_t : public std::pair< A, A >
{
    /** 
     * constructor, call pair constructor
     */
    ptree_t() : std::pair< A, A >()
    {
        /** nothing else to do here **/
    }
   
    virtual ~ptree_t() = default;

    /**
     * get_rhs - basically rename pair.second
     */
    A& get_rhs()
    {
        return( (this).second );
    }

    A& get_lhs()
    {
        return( (this).first );
    }
};

public:
    parsetree() = default;
    virtual ~parseatree() = default;

    /** 
     * pushRHS - push a frontier to the RHS of the parse tree
     * returns false if the existing RHS hasn't been popped, which
     * implies that popRHS must be called if this tree already has
     * a RHS. 
     * @param   frontier_t &&f
     * @return  bool, true if pushed, false if not. 
     */
    bool pushRHS( frontier_t &&f );
    /**
     * popRHS - pop a frontier from the RHS, you must call is_RHS
     * before calling this function to check to see if the RHS 
     * has been initialized to pop in the first place. 
     * @return frontier_t - std::unique_ptr
     */
    frontier_t    popRHS();

    /** 
     * pushLHS - push a frontier to the LHS of the parse tree
     * returns false if the existing LHS hasn't been popped, which
     * implies that popLHS must be called if this tree already has
     * a LHS. 
     * @param   frontier_t &&f
     * @return  bool, true if pushed, false if not. 
     */
    bool pushLHS( frontier_t &&f );
    /**
     * popLHS - pop a frontier from the RHS, you must call is_RHS
     * before calling this function to check to see if the LHS 
     * has been initialized to pop in the first place. 
     * @return frontier_t - std::unique_ptr
     */
    frontier_t    popLHS();
    /**
     * is_RHS - returns true if the RHS is initialized. 
     * @return bool, true if initialized
     */
    bool is_RHS();
    /**
     * is_LHS - returns true if the LHS is initialized. 
     * @return bool, true if initialized
     */
    bool is_LHS();

private:
    ptree_t< frontier_t >   parse_tree  = { nullptr, nullptr };

};

} /** namespace raft **/

#endif /* END _PARSETREE_HPP_ */
