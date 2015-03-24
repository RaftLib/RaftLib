/**
 * kernel.hpp - 
 * @author: Jonathan Beard
 * @version: Thu Sep 11 15:34:24 2014
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
#ifndef _KERNEL_HPP_
#define _KERNEL_HPP_  1

#include <functional>
#include <utility>
#include <setjmp.h>

#include "port.hpp"
#include "signalvars.hpp"
#include "rafttypes.hpp"

class MapBase;
class Schedule;

#ifndef CLONE
namespace raft
{
   class kernel;
}

#define CLONE() \
virtual raft::kernel* clone()\
{\
   return( new typename std::remove_reference< decltype( *this ) >::type( ( *(\
   (typename std::decay< decltype( *this ) >::type * ) \
   this ) ) ) );\
}
#endif

namespace raft {
class kernel
{
public:
   /** default constructor **/
   kernel();
   
   /** in-place allocation **/
   kernel( void * const ptr,
           const std::size_t nbytes );

   virtual ~kernel() = default;


   /**
    * run - function to be extended for the actual execution.
    * Code can be executed outside of the run function, i.e., 
    * with any function call, however the scheduler will only 
    * call the run function so it must initiate any follow-on
    * behavior desired by the user.
    */
   virtual raft::kstatus run() = 0;
   

   template < class T /** kernel type **/,
               class ... Args >
      static kernel* make( Args&&... params )
      {
         return( new T( std::forward< Args >( params )... ) );
      }
   
   /** 
    * clone - used for parallelization of kernels, if necessary 
    * sub-kernels should include an appropriate copy 
    * constructor so all class member variables can be
    * set.
    * @param   other, T& - reference to object to be cloned
    * @return  kernel*   - takes base type, however is same as 
    * allocated by copy constructor for T.
    */
   virtual raft::kernel* clone()
   {
      //FIXME, needs to throw an exception
      assert( false );
      return( nullptr );
   }

   std::size_t get_id();
protected:
   /**
    * PORTS - input and output, use these to interact with the
    * outside world.
    */
   Port               input  = { this };
   Port               output = { this };
 

   friend class ::MapBase;
   friend class ::Schedule;
   friend class ::GraphTools;
   
   /**
    * NOTE: doesn't need to be atomic since only one thread
    * will have responsibility to to create new compute 
    * kernels.
    */
   static std::size_t kernel_count;

private:
   const  std::size_t kernel_id;
   /**
    * NOTE: this is only accessible to the run-time tools, 
    * we need it to preempt the kernel if we're in a blocked
    * function that has gone on for far too long.  Not a problem
    * when using the OS scheduler, but an issue when we use
    * a pool scheme
    */
   jmp_buf            current_state;
   /**
    * NOTE: previous state is the state as it was before the 
    * kernel was executed.  The scheduler will return the 
    * kernel to it's "current_state" on the next invocation 
    * of the compute kernel.
    */
   jmp_buf            prev_state;
};
} /** end namespace raft */
#endif /* END _KERNEL_HPP_ */
