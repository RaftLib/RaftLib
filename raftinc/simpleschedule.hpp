/**
 * simpleschedule.hpp - 
 * @author: Jonathan Beard
 * @version: Thu Sep 11 15:49:57 2014
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
#ifndef RAFTSIMPLESSCHEDULE_HPP
#define RAFTSIMPLESSCHEDULE_HPP  1
#include <vector>
#include <thread>
#include <cstdint>
#include "defs.hpp"

namespace raft{
   class kernel;
   class map;
}

class simple_schedule : public Schedule
{
public:
    simple_schedule( raft::map &map );

    virtual ~simple_schedule() override;

    virtual void init() override;
    
    virtual void start() override;
   
protected:

    virtual void handleSchedule( raft::kernel * const kernel ) override;
                                 
    static void simple_run( void  *data );

    struct thread_data
    {
       constexpr thread_data( raft::kernel * const k,
                              bool *fin ) : k( k ),
                                            finished( fin ){}

       raft::kernel *k         = nullptr;
       bool         *finished  = nullptr;
       core_id_t     loc       = -1;
    };
    
    struct thread_info_t
    {
       thread_info_t( raft::kernel * const kernel ) : data( kernel, 
                                                            &finished ),
                                      th( simple_run,
                                          reinterpret_cast< void* >( &data ) )
       {
       }

       bool           finished = false;
       bool           term     = false;
       thread_data    data;
       std::thread    th;
    };

    
    std::mutex                    thread_map_mutex;
    std::vector< thread_info_t* > thread_map;
    bool keep_going  = true;
};
#endif /* END RAFTSIMPLESSCHEDULE_HPP */
