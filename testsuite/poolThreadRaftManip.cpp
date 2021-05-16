/**
 * allocateSendPush.cpp - throw an error if internal object
 * pop fails.
 *
 * @author: Jonathan Beard
 * @version: Sat Feb 27 19:10:26 2016
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
#include <cstdint>
#include <cstdio>
#include <cstddef>
#include <raft>
#include <raftmanip>
#include <cstdlib>
#include <cassert>

#include "streamManipUtils.h"


int
main()
{
    start   s;
    middle  m;
    last    l;
    
    using group_one = raft::parallel::affinity_group<1>;
    using group_two = raft::parallel::affinity_group<2>;

    using cpu_one   = raft::parallel::device< raft::parallel::cpu, 1 >;
    //just realized CI service is using single core, so, this will fault. 
    //using cpu_two   = raft::parallel::device< raft::parallel::cpu, 2 >;

    /**
     * just set affinity as a group, let the runtime figure
     * out which core to assign this affinity group to.
     */
    auto group_one_container = raft::manip< group_one, cpu_one >::bind( l, m ); 
    //just for kicks, let's print this out
    for( auto &x : group_one_container )
    {
        std::cout << x.get().get_id() << " - " << x.get().getCoreAssignment() << "\n"; 
    }
    raft::manip< group_two, cpu_one >::bind( s ); 
    
    /**
     * set affinity, but provide a modifier in the arguments 
     * to force this affinity group to a specific core, it
     * assumes the programmer konws exactly what they're doing. 
     * we also provide a means to set to a named node as an 
     * expansion point to distributed systems. 
     */
    //raft::manip< raft::parallel::affinity,
    //             raft::parallel::force_core( 0 ) >::bind( 
    //    l, m                            /** two kernels in group **/ ); 

    raft::map M;
    M += s >> m >> l;
    M.exe();
    return( EXIT_SUCCESS );
}
