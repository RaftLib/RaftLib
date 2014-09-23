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


namespace order
{
   enum spec  { in, out };
}

class Map
{
public:
   Map();
   virtual ~Map();
   

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
            PortInfo port_info_a;
            try{ 
               port_info_a =  a->output.getPortInfo();
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
            PortInfo port_info_b;
            try{
               port_info_b = b->input.getPortInfo();
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
            
            join( *a, port_info_a.my_name, port_info_a, 
                  *b, port_info_b.my_name, port_info_b );
            
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
            auto port_info_a( a->output.getPortInfoFor( a_port ) );
            
            PortInfo port_info_b;
            try{
               port_info_b = b->input.getPortInfo();
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
                  *b, port_info_b.my_name, port_info_b );

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
            PortInfo port_info_a;
            try{
               port_info_a = a->output.getPortInfo();
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
            
            auto port_info_b( b->input.getPortInfoFor( b_port) );
            
            join( *a, port_info_a.my_name, port_info_a, 
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

   template< class scheduler = simple_schedule, class allocator = stdalloc > 
      void exe()
   {
      checkEdges( source_kernels );
      allocator alloc( this );
      /** launch allocator in a thread **/
      std::thread mem_thread( [&](){
         alloc.run();
      });
      
      scheduler sched( this );
      /** launch scheduler in thread **/
      std::thread sched_thread( [&](){
         sched.start();
      });
      
      /** join scheduler first **/
      sched_thread.join();
      mem_thread.join();
   }


private:
   
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
   
   
   /** need to keep source kernels **/
   std::set< Kernel* > source_kernels;
   /** and keep a list of all kernels **/
   std::set< Kernel* > all_kernels; 
   

   friend class Schedule;
   friend class Allocate;
};
#endif /* END _MAP_HPP_ */
