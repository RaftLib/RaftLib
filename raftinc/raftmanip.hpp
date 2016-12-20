/**
 * raftmanip.hpp - 
 * @author: Jonathan Beard
 * @version: Tue Aug 16 09:56:56 2016
 * 
 * Copyright 2016 Jonathan Beard
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
#ifndef _RAFTMANIP_HPP_
#define _RAFTMANIP_HPP_  1

#include <exception>
#include <string>
#include <climit>
#include <cassert>

namespace raft
{

/** 
 * type for stream manipulation, currently
 * this means that there are 64 available
 * modifiers.
 */

using manip_vec_t = std::uint64_t;

/** raft::parallel **/
namespace parallel
{
    enum type : manip_vec_t { system = 0  /** do whatever the runtime wants, I don't care  **/,
                              thread      /** specify a thread for each kernel **/, 
                              pool        /** thread pool, one kernel thread per core, many kernels in each **/, 
                              process     /** open a new process from this point **/,
                              NPARALLEL };
    
}
/** raft::vm **/
namespace vm
{
    enum type { flat = NPARALLEL        /** not yet implemented, likely using segment  **/, 
                standard                /** threads share VM space, processes have sep **/, 
                partition               /** partition graph at this point into a new VM space, platform dependent **/ }; 
}


template < manip_vec_t... VECLIST > struct manip_helper{};

template <> struct manip_helper<>
{
    constexpr static manip_vec_t get_value()
    {
        return( static_cast< manip_vac_t >( 0 ) );
    }
};

template < manip_vec_t N, manip_vec_t... VECLIST > struct manip_helper< N, VECLIST... >
{
    constexpr static manip_vec_t get_value( )
    {
        static_assert( N <= sizeof( manip_vec_t ) * CHAR_BIT, "Stream manipulator can only have 64 states [0:63], please check code" );
        return( ( static_cast< manip_vec_t >( 1 ) << N ) | manip_helper< VECLIST... >::get_value() ); 
    }
};


template < manip_vec_t... VECLIST > struct manip
{
    constexpr static manip_vec_t value      = manip_helper< VECLIST... >::get_value();
};

} /** end namespace raft **/


class RaftManipException : public std::exception
{
public:
   RaftManipException( const std::string message ) : message( message ){};
   virtual const char* what() const noexcept;
private:
   std::string message;
};

class NonsenseChainRaftManipException : public RaftManipException
{
public:
    NonsenseChainRaftManipException( const raft::parallel::type a,
                                     const raft::parallel::type b );
};

#endif /* END _RAFTMANIP_HPP_ */
