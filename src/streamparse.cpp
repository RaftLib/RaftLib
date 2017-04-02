/**
 * streamparse.cpp - 
 * @author: Jonathan Beard
 * @version: Thu Dec 29 05:52:20 2016
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
#include "streamparse.hpp"
#include "streamparseexception.tcc"

/**
 * Section #1, these are all the "starter
 * kernels where the parsemap is first constructed
 * and dereferenced.
 */
raft::parsemap_ptr operator >> ( raft::kernel &src, raft::kernel &dst )
{
    auto parsemap_ptr( std::make_unique< raft::parsemap >( ) );
    parsemap_ptr->parse_link( &src, &dst);
    return( parsemap_ptr );
}

raft::parsemap_ptr operator >> ( raft::kernel_wrapper src, raft::kernel &dst )
{
    auto parsemap_ptr( std::make_unique< raft::parsemap >( ) );
    auto *src_ptr( src.get() );
    src.release();
    auto *dst_ptr( &dst );
    parsemap_ptr->parse_link( src_ptr, dst_ptr );
    return( parsemap_ptr );
}

raft::parsemap_ptr operator >> ( raft::kernel &src, raft::kernel_wrapper dst )
{
    auto parsemap_ptr( std::make_unique< raft::parsemap >( ) );
    auto *src_ptr( &src );
    auto *dst_ptr( dst.get() );
    dst.release();
    parsemap_ptr->parse_link( src_ptr, dst_ptr);
    return( parsemap_ptr );
}

raft::parsemap_ptr operator >> ( raft::kernel_wrapper src, raft::kernel_wrapper dst )
{
    auto parsemap_ptr( std::make_unique< raft::parsemap >( ) );
    auto *src_ptr( src.get() );
    auto *dst_ptr( dst.get() );
    src.release();
    dst.release();
    parsemap_ptr->parse_link( src_ptr, dst_ptr);
    return( parsemap_ptr );
}

/**
 * for the continuation with a LHS as a parsemap, there are some
 * things we have to be careful with. If:
 * LHS = multiple groups, we have to duplicate dst for each.
 * LHS = just a kernel, then no duplication, just a continue.
 */
raft::parsemap_ptr operator >> ( raft::parsemap_ptr src ,   raft::kernel &dst           )
{
    src->parse_link_continue( &dst );    
    return( src );
}

raft::parsemap_ptr operator >> ( raft::parsemap_ptr src ,   raft::kernel_wrapper dst    )
{
    auto *dst_ptr( dst.get() );
    dst.release();
    src->parse_link_continue( dst_ptr );    
    return( src );
}
