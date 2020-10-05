/**
 * makedot.cpp - 
 * @author: Jonathan Beard
 * @version: Sun Oct  4 09:15:09 2020
 * 
 * Copyright 2020 Jonathan Beard
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

#include <ostream>
#include <sstream>
#include <ios>
#include <iostream>
#include <cstdlib>
#include <string>

#include "makedot.hpp"
#include "common.hpp"
#include "map.hpp"
#include "graphtools.hpp"
#include "ringbuffertypes.hpp"

raft::make_dot::make_dot( raft::map &map ) : all_kernels( map.all_kernels ),
                                             source_kernels( map.source_kernels )
{
    auto *height_env( std::getenv( "GEN_DOT_HEIGHT" ) );
    if( height_env != nullptr )
    {
        height = std::stoi( height_env );
    }
    auto *width_env( std::getenv( "GEN_DOT_WIDTH" ) );
    if( width_env != nullptr )
    {
        width  = std::stoi( width_env );
    }
}


void 
raft::make_dot::_run( std::ostream &stream )
{
    //make dot header
    generate_preamble( stream );
    //run over all kernels to make header
    generate_vertex_list( stream );
    //run through graph edges to build edge list
    generate_edge_list( stream );
    //dot close format
    generate_close( stream );
    return;
}


void
raft::make_dot::generate_preamble( std::ostream &stream )
{
    stream << "digraph G{\n";
    stream << "\t";
    stream << raft::make_dot::generate_field( "size", height, width ) << ";\n"; 
    return;
}


void 
raft::make_dot::generate_vertex_list( std::ostream &stream )
{
    auto &c( all_kernels.acquire() );
    for( auto &k /** kernel **/ : c /** in container **/ )
    {
        stream << "\t" << k->get_id();
        stream << "[";
        stream << raft::make_dot::generate_field( "label", 
                                                   common::printClassName( *k ) );
        stream << ", ";
        stream << raft::make_dot::generate_field( "shape", "ellipse" ) << ", ";
        stream << raft::make_dot::generate_field( "fontname", "Helvetica" );
        stream << "];\n";
    }
    all_kernels.release();
    return;
}

void 
raft::make_dot::generate_edge_list( std::ostream &stream )
{
    (void)stream;
    auto dot_func = [&](    PortInfo &a,
                            PortInfo &b,
                            void *data ) -> void
    {
        auto *stream_ptr( reinterpret_cast< std::ostream* >( data ) );
        (*stream_ptr) << "\t" << a.my_kernel->get_id() << " -> ";
        (*stream_ptr) << b.my_kernel->get_id();
        (*stream_ptr) << "[";
        std::stringstream ss;
        ss << a.my_name << " to " << b.my_name  << " (";
        ss << common::printClassNameFromStr( a.type.name() ) + ")";
        ss << "\n";
        ss << "OoO=" << std::boolalpha  << a.out_of_order << "\n";
        ss << "custom allocator=" << std::boolalpha << a.use_my_allocator << "\n";
        ss << "queue type=" << Type::type_prints[ a.mem ] << "\n";
        if( a.existing_buffer != nullptr )
        {
            ss << "existing_buffer\n";
            ss << "\tsize: " << a.nitems << "\n";
            ss << "\tstart_offset: " << a.start_index << "\n";
            ss << "\tfixed_buffer_size: " << a.fixed_buffer_size << "\n";
        }
        (*stream_ptr) << raft::make_dot::generate_field( "label", ss.str() );
        (*stream_ptr) << "];\n";
    };

    auto &c( source_kernels.acquire() );
    GraphTools::BFS( c, dot_func, (void*) &stream );
    source_kernels.release();
    return;
}

void 
raft::make_dot::generate_close( std::ostream &stream )
{
    stream << "}\n";
    return;
}

