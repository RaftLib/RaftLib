/**
 * map.hpp - 
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
#ifndef _MAP_HPP_
#define _MAP_HPP_  1
#include <typeinfo>
#include <cassert>
#include <cxxabi.h>
#include <thread>
#include <sstream>

#include "portexception.hpp"
#include "schedule.hpp"
#include "simpleschedule.hpp"
#include "kernel.hpp"
#include "port_info.hpp"
#include "allocate.hpp"
#include "stdalloc.hpp"

/**
 * kernel_pair_t - struct to be returned by map link functions,
 * in order to allow inline "new" construction of kernels that
 * might be re-used in multiple instances.
 */
struct kernel_pair_t
{
   kernel_pair_t( Kernel &a, Kernel &b ) : src( a ),
                                           dst( b )
   {
   }
   Kernel &src;
   Kernel &dst;
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

class Map
{
public:
   /** 
    * Map - constructor, really doesn't do too much at the monent
    * and doesn't really need to.
    */
   Map();
   /** 
    * default destructor 
    */
   virtual ~Map();
   

   /** 
    * link - this comment goes for the next 4 types of link functions,
    * which basically do the exact same thing.  The template function
    * takes a single param order::spec which is exactly as the name
    * implies, the order of the queue linking the two kernels.  The
    * verious functions are needed to specify different ordering types
    * each of these will be commented seperately below.  This function
    * assumes that Kenrel a has only a single output and Kernel b has
    * only a single input otherwise an exception will be thrown.
    * @param   a - Kernel*, src kernel
    * @param   b - Kernel*, dst kernel
    * @throws  AmbiguousPortAssignmentException - thrown if either src or dst have more than 
    *          a single port to link.
    * @return  kernel_pair_t - references to src, dst kernels.
    */
   template < order::spec t = order::in >
      kernel_pair_t link( Kernel *a, Kernel *b )
   {
      /** assume each only has a single input / output **/
      switch( t )
      {
         case( order::in ):
         {
            if( ! a->input.hasPorts() )
            {
               source_kernels.insert( a );   
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
            
         }
         break;
         case( order::out ):
         {
            assert( false );
         }
         break;
      }
      return( kernel_pair_t( *a, *b ) );
   }
   
   /** 
    * link - same as function above save for the following differences:
    * kernel a is assumed to have multiple ports and the one we wish
    * to link with Kernel b is a_port.  Kernel b is assumed to have a
    * single input port to connect otherwise an exception is thrown.
    * @param   a - Kernel *a, can have multiple ports
    * @param   a_port - port within Kernel a to link
    * @param   b - Kernel *b, assumed to have only single input.
    * @throws  AmbiguousPortAssignmentException - thrown if Kernel b has more than
    *          a single input port.
    * @throws  PortNotFoundException - thrown if Kernel a has no port named
    *          a_port.
    * @return  kernel_pair_t - references to src, dst kernels.
    */
   template < order::spec t = order::in > 
      kernel_pair_t link( Kernel *a, const std::string  a_port, Kernel *b )
   {
      switch( t )
      {
         case( order::in ):
         {
            if( ! a->input.hasPorts() )
            {
               source_kernels.insert( a );   
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

         }
         break;
         case( order::out ):
         {
            assert( false );
         }
         break;
      }
      return( kernel_pair_t( *a, *b ) );
   }
  
   /**
    * link - same as above save for the following differences:
    * Kernel a is assumed to have a single output port.  Kernel
    * b is assumed to have more than one input port, within one
    * matching the port b_port.
    * @param   a - Kernel*, with more a single output port
    * @param   b - Kernel*, with input port named b_port
    * @param   b_port - const std::string, input port name.
    * @throws  AmbiguousPortAssignmentException - exception thrown 
    *          if Kernel a has more than a single output port
    * @throws  PortNotFoundException - exception thrown if Kernel b
    *          has no input port named b_port
    * @return  kernel_pair_t - references to src, dst kernels.
    */
   template < order::spec t = order::in >
      kernel_pair_t link( Kernel *a, Kernel *b, const std::string b_port )
   {
      switch( t )
      {
         case( order::in ):
         {
            if( ! a->input.hasPorts() )
            {
               source_kernels.insert( a );   
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
         }
         break;
         case( order::out ):
         {
            assert( false );
         }
         break;
      }
      return( kernel_pair_t( *a, *b ) );
   }
   
   /**
    * link - same as above save for the following differences:
    * Kernel a is assumed to have an output port a_port and 
    * Kernel b is assumed to have an input port b_port.
    * @param   a - Kernel*, with more a single output port
    * @param   a_port - const std::string, output port name
    * @param   b - Kernel*, with input port named b_port
    * @param   b_port - const std::string, input port name.
    * @throws  PortNotFoundException - exception thrown if either kernel
    *          is missing port a_port or b_port.
    * @return  kernel_pair_t - references to src, dst kernels.
    */
   template < order::spec t = order::in >
      kernel_pair_t link( Kernel *a, const std::string a_port, 
                          Kernel *b, const std::string b_port )
   {
      switch( t )
      {
         case( order::in ):
         {
            if( ! a->input.hasPorts() )
            {
               source_kernels.insert( a );   
            }
            all_kernels.insert( a );
            all_kernels.insert( b );
            auto port_info_a( a->output.getPortInfoFor( a_port ) );
            auto port_info_b( b->input.getPortInfoFor( b_port) );
            
            join( *a, a_port, port_info_a, 
                  *b, b_port, port_info_b );
         }
         break;
         case( order::out ):
         {
            assert( false );
         }
         break;
      }
      return( kernel_pair_t( *a, *b ) );
   }

   /**
    * exe - template function that which takes two template parameters,
    * scheduler and allocator.  Currently the default is simple_schedule
    * and allocator is stdalloc.  The first thing this function does is 
    * check each edge within the streaming graph to ensure that all 
    * edges are connected.  If any unconnected edges have been found
    * then an exception is thrown.  If all edges are connected then the 
    * allocator and scheduler are called in turn based on the template 
    * parameters.
    * @throws PortDoubleInitializeException - thrown if an edge is double
    *         allocated for some reason.
    * @throws PortException - thrown if an unconnected edge is found.
    */ 
   template< class scheduler = simple_schedule, class allocator = stdalloc > 
      void exe()
   {
      checkEdges( source_kernels );
      allocator alloc( (*this) );
      /** launch allocator in a thread **/
      std::thread mem_thread( [&](){
         alloc.run();
      });
     
      alloc.waitTillReady();

      scheduler sched( (*this) );
      sched.init();
      /** launch scheduler in thread **/
      std::thread sched_thread( [&](){
         sched.start();
      });
      
      /** join scheduler first **/
      sched_thread.join();
      mem_thread.join();
      /** all fifo's deallocated when alloc goes out of scope **/
      return; 
   }


private:
  
   /**
    * checkEdges - runs a breadth first search through the graph
    * to look for disconnected edges.
    * @param   source_k - std::set< Kernel* >
    * @throws PortException - thrown if an unconnected edge is found.
    */
   void checkEdges( std::set< Kernel* > &source_k );

   /**
    * join - helper method joins the two ports given the correct 
    * information.  Essentially the correct information for the 
    * PortInfo object is set.  Type is also checked using the 
    * typeid information.  If the types aren't the same then an
    * exception is thrown.
    * @param a - Kernel&
    * @param name_a - name for the port on kernel a
    * @param a_info - PortInfo struct for kernel a
    * @param b - Kernel&
    * @param name_b - name for port on kernel b
    * @param b_info - PortInfo struct for kernel b
    * @throws PortTypeMismatchException
    */
   void join( Kernel &a, const std::string name_a, PortInfo &a_info, 
              Kernel &b, const std::string name_b, PortInfo &b_info );
   
   
   void printEdges( std::set< Kernel* > &source_k );

   /** need to keep source kernels **/
   std::set< Kernel* > source_kernels;
   /** and keep a list of all kernels **/
   std::set< Kernel* > all_kernels; 
   

   friend class Schedule;
   friend class Allocate;
};
#endif /* END _MAP_HPP_ */
