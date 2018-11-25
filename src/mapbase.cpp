/**
 * mapbase.cpp - 
 * @author: Jonathan Beard
 * @version: Fri Sep 12 10:28:33 2014
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
#include <sstream>
#include <map>
#include <sstream>
#include <vector>
#include <typeinfo>
#include <cstdio>
#include <cassert>
#include <string>
#include <cstdlib>
#include <cstring>

#include "common.hpp"
#include "mapbase.hpp"
#include "graphtools.hpp"
#include "demangle.hpp"

MapBase::MapBase()
{
  
}

MapBase::~MapBase()
{
   auto &container( all_kernels.acquire() );
   for( raft::kernel *kern : container )
   {
      if( kern != nullptr && kern->internal_alloc ) 
      {   
         if( kern->internal_alloc )
         {
            delete( kern );
         }
      }
   }
   all_kernels.release();
}
    
kernel_pair_t 
MapBase::link( raft::kernel *a, 
               raft::kernel *b,
               const raft::order::spec t,
               const std::size_t buffer )
{
   updateKernels( a, b );
   PortInfo *port_info_a;
   try{ 
      port_info_a =  &(a->output.getPortInfo());
   }
   catch( AmbiguousPortAssignmentException &ex )
   {
      portNotFound( true,
                    ex,
                    a );
   }
   port_info_a->fixed_buffer_size = buffer;
   PortInfo *port_info_b;
   try{
      port_info_b = &(b->input.getPortInfo());
   }
   catch( AmbiguousPortAssignmentException &ex )
   {
         portNotFound( false, 
                       ex,
                       b );
   }
   port_info_b->fixed_buffer_size = buffer;

   join( *a, port_info_a->my_name, *port_info_a, 
         *b, port_info_b->my_name, *port_info_b );
   set_order( *port_info_a, *port_info_b, t ); 
   return( kernel_pair_t( a, b ) );
}

kernel_pair_t 
MapBase::link( raft::kernel *a, 
               const raft::port_key_type  a_port, 
               raft::kernel *b,
               const raft::order::spec t,
               const std::size_t buffer )
{
   updateKernels( a, b );
   PortInfo &port_info_a( a->output.getPortInfoFor( a_port ) );
   port_info_a.fixed_buffer_size = buffer;
   PortInfo *port_info_b;
   try{
      port_info_b = &(b->input.getPortInfo());
   }
   catch( AmbiguousPortAssignmentException &ex )
   {
         portNotFound( false,
                       ex,
                       b );
   }
   port_info_b->fixed_buffer_size = buffer;
   join( *a, a_port , port_info_a, 
         *b, port_info_b->my_name, *port_info_b );
   set_order( port_info_a, *port_info_b, t ); 
   return( kernel_pair_t( a, b ) );
}

kernel_pair_t 
MapBase::link( raft::kernel *a, 
               raft::kernel *b, 
               const raft::port_key_type b_port,
               const raft::order::spec t,
               const std::size_t buffer )
{
   updateKernels( a, b );
   PortInfo *port_info_a;
   try{
      port_info_a = &(a->output.getPortInfo() );
   }
   catch( AmbiguousPortAssignmentException &ex )
   {
         portNotFound( true,
                       ex,
                       a );
   }
   port_info_a->fixed_buffer_size = buffer;
   
   PortInfo &port_info_b( b->input.getPortInfoFor( b_port) );
   port_info_b.fixed_buffer_size = buffer;
   
   join( *a, port_info_a->my_name, *port_info_a, 
         *b, b_port, port_info_b );
   set_order( *port_info_a, port_info_b, t ); 
   return( kernel_pair_t( a, b ) );
}
 
kernel_pair_t
MapBase::link( raft::kernel *a, 
               const raft::port_key_type a_port, 
               raft::kernel *b, 
               const raft::port_key_type b_port,
               const raft::order::spec t,
               const std::size_t buffer )
{
   updateKernels( a, b );
   auto &port_info_a( a->output.getPortInfoFor( a_port ) );
   port_info_a.fixed_buffer_size = buffer;
   auto &port_info_b( b->input.getPortInfoFor( b_port) );
   port_info_b.fixed_buffer_size = buffer;
   
   join( *a, a_port, port_info_a, 
         *b, b_port, port_info_b );
   set_order( port_info_a, port_info_b, t ); 
   return( kernel_pair_t( a, b ) );
}


void
MapBase::join( raft::kernel &a, const raft::port_key_type &name_a, PortInfo &a_info, 
               raft::kernel &b, const raft::port_key_type &name_b, PortInfo &b_info )
{
   //b's port info isn't allocated
   if( a_info.type != b_info.type )
   {
      std::stringstream ss;
      ss << "When attempting to join ports (" << common::printClassName( a ) <<  
         "[" << name_a << "] -> " << common::printClassName( b ) << "[" << 
         name_b << "] have conflicting types.  " << 
            common::printClassNameFromStr( a_info.type.name() ) <<  
         " and " << common::printClassNameFromStr( b_info.type.name() ) << "\n"; 
      throw PortTypeMismatchException( ss.str() );
   }

   a_info.other_kernel = &b;
   a_info.other_name   = name_b;
   b_info.other_kernel = &a;
   b_info.other_name   = name_a;
}
   
void 
MapBase::insert( raft::kernel *a,  PortInfo &a_out, 
                 raft::kernel *b,  PortInfo &b_in,
                 raft::kernel *i)
{
   PortInfo &i_in( i->input.getPortInfoFor( "0" ) ),
            &i_out( i->output.getPortInfoFor( "0" ) );
   join( *a, a_out.my_name, a_out,
         *i, i_in.my_name, i_in );
   
   join( *i, i_out.my_name, i_out,
         *b, b_in.my_name, b_in );
}
   
   
void 
MapBase::set_order( PortInfo &port_info_a, 
                    PortInfo &port_info_b,
                    const raft::order::spec t ) noexcept
{
     const bool value(t == raft::order::out ? true : false );
     port_info_a.out_of_order = value; 
     port_info_b.out_of_order = value;
     return;           
}
   
void 
MapBase::updateKernels( raft::kernel * const a, raft::kernel * const b )
{
   if( ! a->input.hasPorts() )
   {
      source_kernels += a;
   }
   if( ! b->output.hasPorts() )
   {
      dst_kernels += b;
   }
   all_kernels += a;
   all_kernels += b;
   return;
}
   
void 
MapBase::portNotFound( const bool src, 
                       const AmbiguousPortAssignmentException &ex,
                       raft::kernel * const k )
{
    std::stringstream ss;
    const auto name( raft::demangle( typeid( *k ).name() ) );
    if( src )
    {
       ss << ex.what() << "\n";
       ss << "Output port from source kernel (" << name << ") " <<
              "has more than a single port.";
    
    }
    else
    {
       ss << ex.what() << "\n";
       ss << "Input port from destination kernel (" << name << ") " <<
              "has more than a single port.";
    }
    throw AmbiguousPortAssignmentException( ss.str() );
    return;
}
