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
template < class... PORTS >
   class lambdak : public raft::kernel
{
public:
   typedef std::function< raft::kstatus ( Port &input, Port &output ) > lambdafunc_t;
   lambdak( const std::size_t inputs, 
            const std::size_t outputs, 
            lambdafunc_t  &&func ) : raft::kernel(),
                                     run_func( func )
   {
      const auto num_types( sizeof... (PORTS) );
      if( num_types == 1 )
      {
         /** everybody gets same type, add here **/
         AddSamePorts< PORTS... >::add( inputs /* count */, 
                                        input  /* in-port obj */,
                                        outputs /* count */,
                                        output /* out-port obj */ );
      }
      /** no idea what type each port is, throw error **/
      else if( num_types != (inputs + outputs ) ) 
      {
         /** TODO, make exception for here **/
         assert( false );
      }
      /** call recursive add function **/
      add_ports< Ports... >::add( inputs, outputs );
   }

   virtual raft::kstatus run()
   {
      return( run_func( input  /** input ports **/, 
                        output /** output ports **/) ); 
   }


private:

   lambdafunc_t  run_func;

   
   /** function **/
   template < class... PORTS >
      void add_ports( const std::size_t input_max, 
                      const std::size_t output_max )
   {
      std::size_t input_index(  0 );
      std::size_t output_index( 0 );
      AddPorts< PORTS >::add( input_index,
                              input_max, 
                              input /* ports */, 
                              output_index,
                              output_max, 
                              output /* ports */);
   }

   template < class... PORTS > struct AddPorts;

   /** class recursion **/
   template < class PORT, class... PORTS > struct AddPorts< PORT, PORTS...>
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
            AddPorts< PORTS >::add( input_index,
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
            AddPorts< PORTS >::add( input_index,
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
      static void add( std::size_t &input_index,
                       const std::size_t input_max,
                       Port &input_ports,
                       std::size_t &output_index,
                       const std::size_t output_max,
                       Port &output_ports )
      {
         return;
      }
   };


   /** single class type, no recursion **/
   template < class PORT > struct AddSamePorts
   {
      static void add( const std::size_t input_count, Port &inputs,
                       const std::size_t output_count, Port &outputs )
      {
         for( auto it( 0 ); it < input_count; it++ )
         {
            inputs.addPort< PORT >( std::to_string( it ) );
         }
         for( auto it( 0 ); it < output_count; it++ )
         {
            outputs.addPort< PORT >( std::to_string( it ) );
         }
      }
   };

}; /** end template lambdak **/

} /* end namespace raft */
#endif /* END _LAMBDAK_TCC_ */
