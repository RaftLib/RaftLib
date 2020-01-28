/**
 * parsetree.cpp - 
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
#include <cassert>
#include "parsetree.hpp"
    
    
bool
raft::parsetree::pushRHS( frontier_t &&f )
{
    /**
     * return false if there is already a RHS
     */
    if( is_RHS() )
    {
        return( false );
    }
    assert( parse_tree.get_rhs() == nullptr );
    auto &old_rhs( parse_tree.get_rhs() );
    old_rhs = std::move( f );
    return( true );
}


frontier_t  
raft::parsetree::popRHS()
{
    /** 
     * is_rhs should have been called by this point 
     */
    assert( parse_tree.get_rhs() == nullptr );
    auto old_rhs( raft::make_frontier_t() );
    old_rhs = /** get copy **/ std::move( parse_tree.get_rhs() );
    parse_tree.get_rhs() = nullptr;
    return( old_rhs );
}

bool
raft::parsetree::pushLHS( frontier_t &&f )
{
    /**
     * return false if there is already a LHS
     */
    if( is_LHS() )
    {
        return( false );
    }
    assert( parse_tree.get_lhs() == nullptr );
    auto &old_lhs( parse_tree.get_lhs() );
    old_lhs = std::move( f );
    return( true );
}

frontier_t 
raft::parsetree::popLHS()
{
    /** 
     * is_lhs should have been called by this point 
     */
    assert( parse_tree.get_lhs() == nullptr );
    auto old_lhs( raft::make_frontier_t() );
    old_lhs = /** get copy **/ std::move( parse_tree.get_lhs() );
    parse_tree.get_lhs() = nullptr;  
    return( old_lhs );
}

bool 
raft::parsetree::is_RHS()
{
    if( parse_tree.get_rhs() == nullptr )
    {
        return( false );
    }
    return( true );
}

bool 
raft::parsetree::is_LHS()
{
    if( parse_tree.get_lhs() == nullptr )
    {
        return( false );
    }
    return( true );
}
