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
#ifndef RAFTKERNEL_HPP
#define RAFTKERNEL_HPP  1

#include <functional>
#include <utility>
#include <cstdint>
#include <queue>
#include <string>
#ifdef BENCHMARK
#include <atomic>
#endif
#include "kernelexception.hpp"
#include "port.hpp"
#include "signalvars.hpp"
#include "rafttypes.hpp"
#include "kernel_wrapper.hpp"

/** pre-declare for friends **/ 
class MapBase;
class Schedule;
class kernel_container;
class Map;
class basic_parallel;
class kpair;
class interface_partition;
class pool_schedule;


#ifndef CLONE
namespace raft
{
   class kernel;
}
#define CLONE() \
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
      static kernel_wrapper make( Args&&... params )
      {
         auto *output( new T( std::forward< Args >( params )... ) );
         output->internal_alloc = true;
         return( kernel_wrapper( output ) );
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
      throw CloneNotImplementedException( "Sub-class has failed to implement clone function, please use the CLONE() macro to add functionality" );
      /** won't be reached **/
      return( nullptr );
   }

   std::size_t get_id();
   
   /**
    * operator[] - returns the current kernel with the 
    * specified port name enabled for linking.
    * @param portname - const raft::port_key_type&&
    * @return raft::kernel&&
    */
#ifdef STRING_NAMES    
   raft::kernel& operator []( const raft::port_key_type &&portname );
   raft::kernel& operator []( const raft::port_key_type &portname );
#else
    template < class T > raft::kernel& 
        operator []( const T &&portname )
    {
       if( enabled_port.size() < 2 )
       {
            enabled_port.push( portname.val );
       }
       else
       {
            throw AmbiguousPortAssignmentException(
            //    "too many ports added with: " + portname.str
                "too many ports added with: "
            );
       }
       return( (*this) );
    }
    
    template < class T > raft::kernel& 
        operator []( const T &portname )
    {
       if( enabled_port.size() < 2 )
       {
            enabled_port.push( portname.val );
       }
       else
       {
            throw AmbiguousPortAssignmentException(
//                "too many ports added with: " + portname.str
                "too many ports added with: "
            );
       }
       return( (*this) );
    }
#endif /** end if not string names **/ 

    core_id_t getCoreAssignment() noexcept
    {
        return( core_assign );
    }
    
    /**
     * PORTS - input and output, use these to interact with the
     * outside world.
     */
    Port               input  = { this };
    Port               output = { this };
    

    constexpr void setCore( const core_id_t id )
    {
        core_assign = id;
        return;
    }
    
    constexpr void setAffinityGroup( const core_id_t ag )
    {
        affinity_group = ag;
        return;
    }

protected:
    /**
     * 
     */
    virtual std::size_t addPort();
    
    void allConnected();
    
    virtual void lock();
    virtual void unlock();

  
    
    raft::port_key_type getEnabledPort();
    
    /** in namespace raft **/
    friend class map;
    /** in global namespace **/
    friend class ::MapBase;
    friend class ::Schedule;
    friend class ::GraphTools;
    friend class ::kernel_container;   
    friend class ::basic_parallel;
    friend class ::kpair;
    friend class ::interface_partition;
    friend class ::pool_schedule;

    /**
     * NOTE: doesn't need to be atomic since only one thread
     * per process will have responsibility to to create new 
     * compute kernels, for multi-process, this is used in 
     * conjunction with process identifier.
     */
    static std::size_t kernel_count;
    
#ifdef BENCHMARK
    static std::atomic< std::size_t > initialized_count;
#endif

    bool internal_alloc = false;

    
    void  retire() noexcept
    {
        (this)->execution_done = true;
    }

    bool isRetired() noexcept
    {
        return( (this)->execution_done );
    }
    



    /**
     * these are both set to -1 by defualt, which
     * means unset. 
     */
    core_id_t   core_assign       = -1;
    core_id_t   affinity_group    = -1;


    raft::schedule_behavior     sched_behav = raft::any_port;
private:
   /** TODO, replace dup with bit vector **/
   bool             dup_enabled       = false;
   bool             dup_candidate     = false;
   const            std::size_t kernel_id;

   bool             execution_done    = false;

   /** for operator syntax **/
   std::queue< raft::port_key_type > enabled_port;
};


} /** end namespace raft */
#endif /* END RAFTKERNEL_HPP */
