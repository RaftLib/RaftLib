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
#include <cstdint>
#include <queue>
#include <string>
#include "defs.hpp"
#include "kernelexception.hpp"
#include "port.hpp"
#include "signalvars.hpp"
#include "rafttypes.hpp"
#include "kernel_wrapper.hpp"
#include "raftmanip.hpp"

/** pre-declare for friends **/ 
class MapBase;
class Schedule;
class kernel_container;
class submap;
class basic_parallel;
class kpair;
class interface_partition;
class pool_schedule;


#ifndef CLONE
namespace raft
{
   class kernel;
}

#define IMPL_CPY_CLONE() \
virtual raft::kernel* clone()\
{ \
   auto *ptr( \
      new typename std::remove_reference< decltype( *this ) >::type( ( *(\
   (typename std::decay< decltype( *this ) >::type * ) \
   this ) ) ) );\
   /** RL needs to dealloc this one **/\
   ptr->internal_alloc = true;\
   return( ptr );\
}

#define IMPL_CLONE() \
virtual raft::kernel* clone()\
{ \
   auto *ptr( \
      new typename std::remove_reference< decltype( *this ) >::type() );\
   /** RL needs to dealloc this one **/\
   ptr->internal_alloc = true;\
   return( ptr );\
}

#define IMPL_NO_CLONE() \
virtual raft::kernel* clone()\
{ \
    return( nullptr );\
}
#define CLONE 1
#endif /** end CLONE impls **/



namespace raft {
/** predeclare template **/
template < manip_vec_t... VECLIST > struct manip;

class kernel
{
using clone_helper = std::function< raft::kernel* () >;

public:
   /** default constructor **/
   kernel();

   kernel( const kernel &other );

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
      static kernel_wrapper make( Args&&... params )
      {
         kernel_wrapper output( new T( std::forward< Args >( params )... ) );
         output->internal_alloc = true;
         return( output );
      }
   
   /** 
    * clone - used for parallelization of kernels, if necessary 
    * sub-kernels should include an appropriate copy 
    * constructor so all class member variables can be
    * set. To implement this in a sub-class see the
    * macros IMPL_CLONE, IMPL_CPY_CLONE, or IMPL_NO_CLONE
    * to simply not implement this function (i.e. return
    * null).
    * @return  kernel*   - takes base type, however is same as 
    * allocated by copy constructor for T. Returns nullptr
    * in the case of IMPL_NO_CLONE
    */
   virtual raft::kernel* clone() = 0;
   
   /** 
    * get_id - returns this kernels id, won't change for the
    * lifetime of this kernel once set.
    */
   std::size_t get_id() const noexcept;
   
   /**
    * operator[] - returns the current kernel with the 
    * specified port name enabled for linking.
    * @param portname - const std::string&&
    * @return raft::kernel&&
    */
   raft::kernel& operator []( const std::string &&portname );

   /**
    * getCoreAssignent - returns the core that this kernel
    * is assigned to. 
    * @return core_id_t
    */
   core_id_t getCoreAssignment() noexcept;

protected:
   /**
    * 
    */
   virtual std::size_t addPort();
   
   virtual void lock();
   virtual void unlock();

   /**
    * PORTS - input and output, use these to interact with the
    * outside world.
    */
   Port               input  = { this };
   Port               output = { this };
  
   
   /** 
    * getEnabledPort - returns the currently enabled port
    * so that the other pieces of the run-time can get the
    * port that was included in the ["xyz"] brackets. Should
    * return a copy, right now this is the behavior I want
    * so that way if we copy compute kernels we won't have
    * multiple kernels potentially pointing to the same reference.
    * Each port name is only returned once, and then never again.
    * This doesn't mean that you are getting rid of the port, it 
    * means that the port name is not going to be returned as "enabled"
    * twice. 
    * @return   std::string - currently active port name
    */
   std::string getEnabledPort();

   /**
    * getEnabledPortCount - returns the number of enabled ports
    * withint this container.
    */
   std::size_t getEnabledPortCount();
   
   /** in namespace raft **/
   friend class map;
   friend class submap;
   friend class parsemap;
   /** in global namespace **/
   friend class ::MapBase;
   friend class ::Schedule;
   friend class ::GraphTools;
   friend class ::kernel_container;   
   friend class ::basic_parallel;
   friend class ::kpair;
   friend class ::interface_partition;
   friend class ::pool_schedule;
   friend void raft::manip_local::apply_help( const manip_vec_t value, 
                                         raft::kernel &k );

   /**
    * NOTE: doesn't need to be atomic since only one thread
    * will have responsibility to to create new compute 
    * kernels.
    */
   static std::size_t kernel_count;
    
   bool internal_alloc = false;

   void setCore( const core_id_t id ) noexcept;
   
   /** 
    * apply - set system settings from stream manipulate
    * or bind calls.
    * @param settings - const raft::manip_vec_t
    */
   void apply( const raft::manip_vec_t settings ) noexcept;


   core_id_t core_assign    = -1;
   
   /** 
    * set to parallel method to default 
    * system and standard for vm 
    */
   manip_vec_t  system_configs  = 
        raft::manip< 
            raft::parallel::system, 
            raft::vm::standard >::value;

private:
   /** TODO, replace dup with bit vector **/
   bool             dup_enabled       = false;
   bool             dup_candidate     = false;
   const            std::size_t kernel_id;

   bool             execution_done    = false;

   /** for operator syntax **/
   std::queue< std::string > enabled_port;
};


} /** end namespace raft */
#endif /* END _KERNEL_HPP_ */
