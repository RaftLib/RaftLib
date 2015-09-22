/**
 * mapbase.hpp - Base map object.  The general idea
 * is that the map object will contain everything needed
 * to construct a streaming topology.  There are two
 * sub-classes.  One is the final "map" topolgoy, the other
 * is the kernel map topolgoy.  The "kernel map" varient
 * has no exe functions nor map checking functions.  It is 
 * assumed that the "kernel map" derivative is contained
 * within a kernel which might have several sub "kernels" 
 * within it.  
 *
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
#ifndef _MAPBASE_HPP_
#define _MAPBASE_HPP_  1
#include <typeinfo>
#include <cassert>
#include <cxxabi.h>
#include <vector>
#include <thread>
#include <sstream>

#include "portexception.hpp"
#include "schedule.hpp"
#include "simpleschedule.hpp"
#include "kernel.hpp"
#include "port_info.hpp"
#include "allocate.hpp"
#include "dynalloc.hpp"
#include "stdalloc.hpp"

/**
 * kernel_pair_t - struct to be returned by mapbase link functions,
 * in order to allow inline "new" construction of kernels that
 * might be re-used in multiple instances.
 */
class kernel_pair_t
{
public:
   kernel_pair_t( raft::kernel *a, raft::kernel *b ) : 
                                           src( a ),
                                           dst( b )
   {
   }

   kernel_pair_t( const kernel_pair_t &other ) : src( other.src ),
                                                 dst( other.dst )
   {
   }
   
   kernel_pair_t& operator= ( const kernel_pair_t &other )
   {
      src = other.src;
      dst = other.dst;
      return( *this );
   }

   raft::kernel& getSrc()
   {
      return( *src );
   }

   raft::kernel& getDst()
   {
      return( *dst );
   }

private:
   raft::kernel *src;
   raft::kernel *dst;
};


/**
 * spec is used when specifying the order of items within the queue,
 * by default in order is specified, in the future out wil be fully
 * implemented and will allow quite a few nice optimizations.
 */
namespace order
{
   enum spec  { in, out };
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
    * link - this comment goes for the next 4 types of link functions,
    * which basically do the exact same thing.  The template function
    * takes a single param order::spec which is exactly as the name
    * implies, the order of the queue linking the two kernels.  The
    * various functions are needed to specify different ordering types
    * each of these will be commented seperately below.  This function
    * assumes that Kernel 'a' has only a single output and raft::kernel 'b' has
    * only a single input otherwise an exception will be thrown.
    * @param   a - raft::kernel*, src kernel
    * @param   b - raft::kernel*, dst kernel
    * @throws  AmbiguousPortAssignmentException - thrown if either src or 
    *          dst have more than 
    *          a single port to link.
    * @return  kernel_pair_t - references to src, dst kernels.
    */
   template < order::spec t = order::in >
      kernel_pair_t link( raft::kernel *a, raft::kernel *b )
   {
      /** assume each only has a single input / output **/
      if( ! a->input.hasPorts() )
      {
         source_kernels.insert( a );   
      }
      if( ! b->output.hasPorts() )
      {
         dst_kernels.insert( b );
      }

	  if (dst_kernels.find(a) != dst_kernels.end()) {
			  dst_kernels.erase(a);
	  }
	  if (source_kernels.find(b) != source_kernels.end()) {
			  source_kernels.erase(b);
	  }

      all_kernels.insert( a );
      all_kernels.insert( b );
      PortInfo *port_info_a;
      try{ 
         port_info_a =  &(a->output.getPortInfo());
      }
      catch( PortNotFoundException &ex )
      {
         int status( 0 );
         std::stringstream ss;
         ss << 
            "Source port from kernel " << 
            abi::__cxa_demangle( typeid( *a ).name(), 0, 0, &status ) <<
            "has more than a single port.";
         
         throw AmbiguousPortAssignmentException( ss.str() );
      }
      PortInfo *port_info_b;
      try{
         port_info_b = &(b->input.getPortInfo());
      }
      catch( PortNotFoundException &ex )
      {
         int status( 0 );
         std::stringstream ss;
         ss << "Destination port from kernel " << 
            abi::__cxa_demangle( typeid( *b ).name(), 0, 0, &status ) <<
            "has more than a single port.";
         throw AmbiguousPortAssignmentException( ss.str() );
      }
      
      join( *a, port_info_a->my_name, *port_info_a, 
            *b, port_info_b->my_name, *port_info_b );
      switch( t )
      {
         case( order::in ):
         {
            port_info_a->out_of_order = false;
            port_info_b->out_of_order = false;
         }
         break;
         case( order::out ):
         {
            port_info_a->out_of_order = true;
            port_info_b->out_of_order = true;
         }
         break;
      }
      return( kernel_pair_t( a, b ) );
   }
   
   /** 
    * link - same as function above save for the following differences:
    * kernel a is assumed to have multiple ports and the one we wish
    * to link with raft::kernel b is a_port.  raft::kernel b is assumed to have a
    * single input port to connect otherwise an exception is thrown.
    * @param   a - raft::kernel *a, can have multiple ports
    * @param   a_port - port within raft::kernel a to link
    * @param   b - raft::kernel *b, assumed to have only single input.
    * @throws  AmbiguousPortAssignmentException - thrown if raft::kernel b has more than
    *          a single input port.
    * @throws  PortNotFoundException - thrown if raft::kernel a has no port named
    *          a_port.
    * @return  kernel_pair_t - references to src, dst kernels.
    */
   template < order::spec t = order::in > 
      kernel_pair_t link( raft::kernel *a, const std::string  a_port, raft::kernel *b )
   {
      if( ! a->input.hasPorts() )
      {
         source_kernels.insert( a );   
      }
      if( ! b->output.hasPorts() )
      {
         dst_kernels.insert( b );
      }

	  if (dst_kernels.find(a) != dst_kernels.end()) {
			  dst_kernels.erase(a);
	  }
	  if (source_kernels.find(b) != source_kernels.end()) {
			  source_kernels.erase(b);
	  }

      all_kernels.insert( a );
      all_kernels.insert( b );
      PortInfo &port_info_a( a->output.getPortInfoFor( a_port ) );
      
      PortInfo *port_info_b;
      try{
         port_info_b = &(b->input.getPortInfo());
      }
      catch( PortNotFoundException &ex ) 
      {
         int status( 0 );
         std::stringstream ss;
         ss << "Destination port from kernel " << 
            abi::__cxa_demangle( typeid( *b ).name(), 0, 0, &status ) <<
            "has more than a single port.";
         throw AmbiguousPortAssignmentException( ss.str() );
      }
      join( *a, a_port , port_info_a, 
            *b, port_info_b->my_name, *port_info_b );
      switch( t )
      {
         case( order::in ):
         {
            port_info_a.out_of_order   = false;
            port_info_b->out_of_order  = false;
         }
         break;
         case( order::out ):
         {
            port_info_a.out_of_order   = true;
            port_info_b->out_of_order  = true;
         }
         break;
      }
      return( kernel_pair_t( a, b ) );
   }


   /**
    * link - same as above save for the following differences:
    * raft::kernel a is assumed to have a single output port.  raft::kernel
    * b is assumed to have more than one input port, within one
    * matching the port b_port.
    * @param   a - raft::kernel*, with more a single output port
    * @param   b - raft::kernel*, with input port named b_port
    * @param   b_port - const std::string, input port name.
    * @throws  AmbiguousPortAssignmentException - exception thrown 
    *          if raft::kernel a has more than a single output port
    * @throws  PortNotFoundException - exception thrown if raft::kernel b
    *          has no input port named b_port
    * @return  kernel_pair_t - references to src, dst kernels.
    */
   template < order::spec t = order::in >
      kernel_pair_t link( raft::kernel *a, raft::kernel *b, const std::string b_port )
   {
      if( ! a->input.hasPorts() )
      {
         source_kernels.insert( a );   
      }
      if( ! b->output.hasPorts() )
      {
         dst_kernels.insert( b );
      }

	  if (dst_kernels.find(a) != dst_kernels.end()) {
			  dst_kernels.erase(a);
	  }
	  if (source_kernels.find(b) != source_kernels.end()) {
			  source_kernels.erase(b);
	  }

      all_kernels.insert( a );
      all_kernels.insert( b );
      PortInfo *port_info_a;
      try{
         port_info_a = &(a->output.getPortInfo() );
      }
      catch( PortNotFoundException &ex ) 
      {
         std::stringstream ss;
         int status( 0 );
         ss << "Source port from kernel " << 
            abi::__cxa_demangle( typeid( *a ).name(), 0, 0, &status ) <<
            "has more than a single port.";
         throw AmbiguousPortAssignmentException( ss.str() );
      }
      
      PortInfo &port_info_b( b->input.getPortInfoFor( b_port) );
      
      join( *a, port_info_a->my_name, *port_info_a, 
            *b, b_port, port_info_b );
      switch( t )
      {
         case( order::in ):
         {
            port_info_a->out_of_order   = false;
            port_info_b.out_of_order    = false;
         }
         break;
         case( order::out ):
         {
            port_info_a->out_of_order   = true;
            port_info_b.out_of_order    = true;
         }
         break;
      }
      return( kernel_pair_t( a, b ) );
   }
   
   /**
    * link - same as above save for the following differences:
    * raft::kernel a is assumed to have an output port a_port and 
    * raft::kernel b is assumed to have an input port b_port.
    * @param   a - raft::kernel*, with more a single output port
    * @param   a_port - const std::string, output port name
    * @param   b - raft::kernel*, with input port named b_port
    * @param   b_port - const std::string, input port name.
    * @throws  PortNotFoundException - exception thrown if either kernel
    *          is missing port a_port or b_port.
    * @return  kernel_pair_t - references to src, dst kernels.
    */
   template < order::spec t = order::in >
      kernel_pair_t link( raft::kernel *a, const std::string a_port, 
                          raft::kernel *b, const std::string b_port )
   {
      if( ! a->input.hasPorts() )
      {
         source_kernels.insert( a );   
      }
      if( ! b->output.hasPorts() )
      {
         dst_kernels.insert( b );
      }

	  if (dst_kernels.find(a) != dst_kernels.end()) {
			  dst_kernels.erase(a);
	  }
	  if (source_kernels.find(b) != source_kernels.end()) {
			  source_kernels.erase(b);
	  }

      all_kernels.insert( a );
      all_kernels.insert( b );
      auto &port_info_a( a->output.getPortInfoFor( a_port ) );
      auto &port_info_b( b->input.getPortInfoFor( b_port) );
      
      join( *a, a_port, port_info_a, 
            *b, b_port, port_info_b );
      switch( t )
      {
         case( order::in ):
         {
            port_info_a.out_of_order = false; 
            port_info_b.out_of_order = false; 
         }
         break;
         case( order::out ):
         {
            port_info_a.out_of_order = true; 
            port_info_b.out_of_order = true; 
         }
         break;
      }
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
   void join( raft::kernel &a, const std::string name_a, PortInfo &a_info, 
              raft::kernel &b, const std::string name_b, PortInfo &b_info );
   

   /** need to keep source kernels **/
   std::set< raft::kernel* > source_kernels;
   /** dst kernels **/
   std::set< raft::kernel* > dst_kernels;
   /** and keep a list of all kernels **/
   std::set< raft::kernel* > all_kernels; 
   /** flatten these kernels into main map once we run **/
   std::vector< MapBase* >   sub_maps;

   friend class Map;
};
#endif /* END _MAPBASE_HPP_ */
