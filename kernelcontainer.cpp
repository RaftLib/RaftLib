/**
 * kernelcontainer.cpp - 
 * @author: Jonathan Beard
 * @version: Sun Mar 22 09:13:32 2015
 * 
 * Copyright 2015 Jonathan Beard
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
#include <vector>

#include <cassert>
#include "schedule.hpp"
#include "kernelcontainer.hpp"
#include "kernel.hpp"

kernel_container::kernel_container() 
{
   input_buff  = new buffer( 100 );
   output_buff = new buffer( 100 );
}

kernel_container::kernel_container( const std::size_t N ) 
{
   input_buff  = new buffer( N );
   output_buff = new buffer( N );
}


kernel_container::~kernel_container()
{
   delete( input_buff );
   delete( output_buff );
}


kernel_container::buffer&
kernel_container::getInputQueue()
{
   assert( input_buff != nullptr );
   return( *input_buff );
}

kernel_container::buffer&
kernel_container::getOutputQueue()
{
   assert( output_buff != nullptr );
   return( *output_buff );
}

void
kernel_container::container_run( kernel_container &container )
{
   bool shutdown( false );
   auto &input_buffer( container.getInputQueue() );
   auto &output_buffer( container.getOutputQueue() );
   while( ! shutdown )
   {
      while( container.getInputQueue().size() > 0 )
      {
         auto &new_cmd( input_buffer.peek< sched_cmd_t >() );
         switch( new_cmd.cmd )
         {
            case( schedule::add ):
            {
               assert( new_cmd.kernel != nullptr );
               
               const auto ret_val( raft::kernel::setPreemptState( new_cmd.kernel ) );
               switch( ret_val )
               {
                  case( 0 /* newly scheduled kernel */ ):
                  {
                     bool done( false );
                     auto &out_cmd( output_buffer.allocate< sched_cmd_t >() );
                     Schedule::kernelRun( new_cmd.kernel, done );
                     out_cmd.cmd            = ( done ? schedule::kernelfinished : 
                                                       schedule::reschedule );
                     out_cmd.kernel         = new_cmd.kernel;
                     output_buffer.send();
                  }
                  break;
                  case( 1 /* kernel preempted */ ):
                  {
                     assert( false );
                  }
                  break;
                  case( 2 /* kernel prempted a second time, leave in container */ ):
                  {
                     assert( false );
                  }
                  break;
                  default:
                     assert( false );
               }
            }
            break;
            case( schedule::shutdown ):
            {
               /** just in case, a sanity check here **/
               shutdown = true;
            }
            break;
            default:
            {
               std::cerr << "Invalid signal: " << 
                  schedule::sched_cmd_str[ new_cmd.cmd ] << "\n";
               assert( false );
            }
         }
         /** clean-up buffer and recycle head of FIFO **/
         input_buffer.unpeek();
         input_buffer.recycle( 1 );
      }
   }
}
