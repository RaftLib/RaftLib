/**
 * mapbase.hpp - Base map object.  The general idea
 * is that the map object will contain everything needed
 * to construct a streaming topology.  There are two
 * sub-classes.  One is the final "map" topology, the other
 * is the kernel map topology.  The "kernel map" variant
 * has no exe functions nor map checking functions.  It is 
 * assumed that the "kernel map" derivative is contained
 * within a kernel which might have several sub "kernels" 
 * within it.  
 *
 * @author: Jonathan Beard
 * @version: 25 May 2020 
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
#ifndef RAFTMAPBASE_HPP
#define RAFTMAPBASE_HPP  1
#include <typeinfo>
#include <cassert>
#include <vector>
#include <thread>
#include <sstream>
#include "defs.hpp"
#include "kernelkeeper.tcc"
#include "portexception.hpp"
#include "schedule.hpp"
#include "simpleschedule.hpp"
#include "kernel.hpp"
#include "port_info.hpp"
#include "allocate.hpp"
#include "dynalloc.hpp"
#include "stdalloc.hpp"
#include "kpair.hpp"
#include "kernel_pair_t.hpp"

namespace raft
{
    class make_dot;
}

class MapBase
{
public:
   /** 
    * MapBase - constructor, really doesn't do too much at the monent
    * and doesn't really need to.
    */
   MapBase();
   /** 
    * default destructor 
    */
   virtual ~MapBase();
   

   
    /**
     * link - same as above save for the following differences:
     * raft::kernel a is assumed to have an output port a_port and 
     * raft::kernel b is assumed to have an input port b_port.
     * @param   a - raft::kernel*, with more a single output port
     * @param   a_port - const raft::port_key_type, output port name
     * @param   b - raft::kernel*, with input port named b_port
     * @param   b_port - const raft::port_key_type, input port name.
     * @throws  PortNotFoundException - exception thrown if either kernel
     *          is missing port a_port or b_port.
     * @return  kernel_pair_t - references to src, dst kernels.
     */
    template < raft::order::spec t = raft::order::in >
       kernel_pair_t link( raft::kernel *a, 
                           raft::port_key_type a_port, 
                           raft::kernel *b, 
                           raft::port_key_type b_port,
                           const std::size_t buffer = 0 )
    {
        updateKernels( a, b );
        /**
         * START src port discovery
         */
        PortInfo *port_info_a( nullptr );
        if( a_port == raft::null_port_value )
        {
            try{ 
               port_info_a =  &(a->output.getPortInfo());
               a_port = port_info_a->my_name;
            }
            catch( AmbiguousPortAssignmentException &ex )
            {
               portNotFound( true,
                             ex,
                             a );
            }
        }
        else
        {
            port_info_a =  &a->output.getPortInfoFor( a_port );
        }
        port_info_a->fixed_buffer_size = buffer;
        /**
         * START dst port discovery
         */
        PortInfo *port_info_b( nullptr );
        if( b_port == raft::null_port_value )
        {
            try{
                port_info_b = &(b->input.getPortInfo());
                b_port = port_info_b->my_name;
            }
            catch( AmbiguousPortAssignmentException &ex )
            {
                portNotFound( false, 
                              ex,
                              b );
            }

        }
        else
        {
            port_info_b = &b->input.getPortInfoFor( b_port );
        }
        port_info_b->fixed_buffer_size = buffer;
        
        assert( port_info_a != nullptr );
        assert( port_info_b != nullptr );

        join( *a, a_port, *port_info_a, 
              *b, b_port, *port_info_b );
        
        set_order< t >( *port_info_a, *port_info_b ); 
        return( kernel_pair_t( a, b ) );
    }



protected:
   /**
    * join - helper method joins the two ports given the correct 
    * information.  Essentially the correct information for the 
    * PortInfo object is set.  Type is also checked using the 
    * typeid information.  If the types aren't the same then an
    * exception is thrown.
    * @param a - raft::kernel&
    * @param name_a - name for the port on kernel a
    * @param a_info - PortInfo struct for kernel a
    * @param b - raft::kernel&
    * @param name_b - name for port on kernel b
    * @param b_info - PortInfo struct for kernel b
    * @throws PortTypeMismatchException
    */
   static void join( raft::kernel &a, const raft::port_key_type name_a, PortInfo &a_info, 
                     raft::kernel &b, const raft::port_key_type name_b, PortInfo &b_info );
   
   static void insert( raft::kernel *a,  PortInfo &a_out, 
                       raft::kernel *b,  PortInfo &b_in,
                       raft::kernel *i );

   /** 
    * set_order - keep redundant code, well, less redundant. 
    * This version handles the in-order settings.
    * @param    port_info_a, PortInfo&
    * @param    port_info_b, PortInfo&
    */
   template < raft::order::spec t,
              typename std::enable_if< t == raft::order::in >::type* = nullptr > 
   static
   void set_order( PortInfo &port_info_a, 
                   PortInfo &port_info_b ) noexcept
   {
        port_info_a.out_of_order = false; 
        port_info_b.out_of_order = false;
        return;           
   }
   
   /** 
    * set_order - keep redundant code, well, less redundant. 
    * This version handles the out-of-order settings.
    * @param    port_info_a, PortInfo&
    * @param    port_info_b, PortInfo&
    */
   template < raft::order::spec t,
              typename std::enable_if< t == raft::order::out >::type* = nullptr > 
   static 
   void set_order( PortInfo &port_info_a, 
                   PortInfo &port_info_b ) noexcept
   {
        port_info_a.out_of_order = true; 
        port_info_b.out_of_order = true; 
        return;
   }

    template < class A, 
               class B >
    void updateKernels( A &a, B &b )
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
        /**
         * fix for issue #77, this should be safe to use
         * here given it's issued before any destructors
         * should have been called.
         */
        if( a->internal_alloc )
        {
            internally_created_kernels += a;
        }
        if( b->internal_alloc )
        {
            internally_created_kernels += b;
        }
    }

   static void portNotFound( const bool src, 
                             const AmbiguousPortAssignmentException &ex, 
                             raft::kernel * const k );

   /** need to keep source kernels **/
   kernelkeeper              source_kernels;
   /** dst kernels **/
   kernelkeeper              dst_kernels;
   /** and keep a list of all kernels **/
   kernelkeeper              all_kernels;
   

   /**
    * bug fix for issue #77, keeping list of 
    * internally created compute kernels, this
    * after a second thought and looking at the 
    * dev branch, this will need to be thread safe
    * so moving to a kernelkeeper object. - jcb 20Nov2018
    */
    kernelkeeper            internally_created_kernels;
   
   /** 
    * FIXME: come up with better solution for enabling online
    * duplication of submaps as a unit.
    *
    * DOES: flatten these kernels into main map once we run 
    */
   std::vector< MapBase* >   sub_maps;
   friend class raft::map;
   friend class raft::make_dot;
};
   

#endif /* END RAFTMAPBASE_HPP */
