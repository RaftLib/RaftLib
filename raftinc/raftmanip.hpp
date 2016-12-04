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

namespace raft
{
    namespace parallel
    {
        enum type { system = 0  /** do whatever the runtime wants, I don't care  **/,
                    thread      /** specify a thread for each kernel **/, 
                    pool        /** thread pool, one kernel thread per core, many kernels in each **/, 
                    process     /** open a new process from this point **/,
                    N };
        
        const std::string  type_name[ N ] = { "raft::parallel::system",
                                              "raft::parallel::thread",
                                              "raft::parallel::pool",
                                              "raft::parallel::process" };
    }
    namespace vm
    {
        enum type { flat        /** not yet implemented **/, 
                    standard    /** threads share VM space, processes have sep **/, 
                    partition   /** partition graph at this point into a new VM space, platform dependent **/ }; 
    }
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
