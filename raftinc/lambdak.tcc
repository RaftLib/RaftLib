/**
 * lambdak.tcc -
 * @author: Jonathan Beard
 * @version: Thu Oct 30 10:12:36 2014
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
#ifndef _LAMBDAK_TCC_
#define _LAMBDAK_TCC_  1
#include <functional>
#include <utility>
#include <typeinfo>
#include <raft>

namespace raft
{
/** TODO, this needs some more error checking before production **/

/** pre-declare recursive struct / functions **/
template < class... PORTSL > struct AddPorts;
template < class... PORTSK > struct AddSamePorts;

template < class... PORTS >
   class lambdak : public raft::kernel
{
public:
   typedef std::function< raft::kstatus ( Port &input,
                                          Port &output ) > lambdafunc_t;
   /**
    * constructor -
    * @param   inputs - const std::size_t number of inputs to the kernel
    * @param   outputs - const std::size_t number of outputs to the kernel
    * @param   func - static or lambda function to execute.
    */
   lambdak( const std::size_t inputs,
            const std::size_t outputs,
            lambdafunc_t  func ) :   raft::kernel(),
                                     run_func( func )
   {
      add_ports< PORTS... >( inputs, outputs );
   }


    //FIXME, add copy constructor


   /**
    * run - implement the run function for this kernel,
    */
   virtual raft::kstatus run()
   {
      return( run_func( input  /** input ports **/,
                        output /** output ports **/) );
   }



private:
   /** lambda func passed by user **/
   lambdafunc_t  run_func;


   /** function **/
   template < class... PORTSM >
      void add_ports( const std::size_t input_max,
                      const std::size_t output_max )
   {
      const auto num_types( sizeof... (PORTSM) );
      if( num_types == 1 )
      {
         /** everybody gets same type, add here **/
         AddSamePorts< PORTSM... >::add( input_max     /* count */,
                                         input         /* in-port obj */,
                                         output_max    /* count */,
                                         output        /* out-port obj */ );
      }
      /** no idea what type each port is, throw error **/
      else if( num_types != (input_max + output_max ) )
      {
         /** TODO, make exception for here **/
         assert( false );
      }
      else
      {
         /** multiple port type case **/
         std::size_t input_index(  0 );
         std::size_t output_index( 0 );
         AddPorts< PORTSM... >::add( input_index,
                                     input_max,
                                     input /* ports */,
                                     output_index,
                                     output_max,
                                     output /* ports */);
      }
   }



}; /** end template lambdak **/

/** class recursion **/
template < class PORT, class... PORTSL > struct AddPorts< PORT, PORTSL...>
{
   static void add( std::size_t &input_index,
                    const std::size_t input_max,
                    Port &input_ports,
                    std::size_t &output_index,
                    const std::size_t output_max,
                    Port &output_ports )
   {
      if( input_index < input_max )
      {
         /** add ports in order, 0,1,2, etc. **/
         input_ports.addPort< PORT >( std::to_string( input_index++ ) );
         AddPorts< PORTSL... >::add( input_index,
                                     input_max,
                                     input_ports,
                                     output_index,
                                     output_max,
                                     output_ports );
      }
      else if( output_index < output_max )
      {
         /** add ports in order, 0, 1, 2, etc. **/
         output_ports.addPort< PORT >( std::to_string( output_index++ ) );
         AddPorts< PORTSL... >::add( input_index,
                                     input_max,
                                     input_ports,
                                     output_index,
                                     output_max,
                                     output_ports );
      }
      else
      {
         /**
          * I think it'll be okay here simply to return, however
          * we might need the blank specialization below
          */
      }
      return;
   }
};

template <> struct AddPorts<>
{
   static void add( std::size_t & __attribute__((__unused__)) input_index,
                    const std::size_t __attribute__((__unused__)) input_max,
                    Port & __attribute__((__unused__)) input_ports,
                    std::size_t & __attribute__((__unused__)) output_index,
                    const std::size_t __attribute__((__unused__)) output_max,
                    Port & __attribute__((__unused__)) output_ports )
   {
      UNUSED( input_index );
      UNUSED( input_max );
      UNUSED( input_ports );
      UNUSED( output_index );
      UNUSED( output_max );
      UNUSED( output_ports );
      return;
   }
};

/** single class type, no recursion **/
template < class PORT, class... PORTSK > struct AddSamePorts< PORT, PORTSK... >
{
   /** no recursion needed here **/
   static void add( const std::size_t input_count, Port &inputs,
                    const std::size_t output_count, Port &outputs )
   {
      using input_index_type = std::remove_const_t<decltype(input_count)>;
      for( input_index_type it( 0 ); it < input_count; it++ )
      {
         inputs.addPort< PORT >( std::to_string( it ) );
      }

      using output_index_type = std::remove_const_t<decltype(input_count)>;
      for( output_index_type it( 0 ); it < output_count; it++ )
      {
         outputs.addPort< PORT >( std::to_string( it ) );
      }
   }
};

template <> struct AddSamePorts<>
{
   static void add( const std::size_t __attribute__((__unused__)) input_count, 
                    Port & __attribute__((__unused__)) inputs,
                    const std::size_t __attribute__((__unused__)) output_count, 
                    Port & __attribute__((__unused__)) outputs )
   { 
       UNUSED( input_count );
       UNUSED( inputs );
       UNUSED( output_count );
       UNUSED( outputs );
       return; 
   }
};

} /* end namespace raft */
#endif /* END _LAMBDAK_TCC_ */
