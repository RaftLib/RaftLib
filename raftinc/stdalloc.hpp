/**
 * stdalloc.hpp - simple allocation, just initializes the FIFO with a
 * fixed size buffer (512 items) with an alignment of 16-bytes.  This
 * can easily be changed by changing the constants below.
 * @author: Jonathan Beard
 * @version: Sat Sep 20 19:56:49 2014
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
#ifndef RAFTSTDALLOC_HPP
#define RAFTSTDALLOC_HPP  1
#include "allocate.hpp"

namespace raft
{
    class map;
}

class stdalloc : public Allocate
{
public:

    /**
     * stdalloc - default constructor, calls base allocate
     * constructor of Allcoate which sets the map object.
     * After setting this object loose in a thread, the notReady()
     * function must be called so that the queue sees fully
     * allocated buffers as opposed ot null objects.
     * @param map - Map&, map with full application
     * @param exit_alloc - bool whose value is set by the map object
     * owning this one.  Controls when the loop within the run thread
     * is exited.
     */
    stdalloc( raft::map &map, volatile bool &exit_alloc ) :
        Allocate( map, exit_alloc ) {}
    /**
     * destructor, doesn't really do much at he moment.
     */
    virtual ~stdalloc() {}
    /**
     * run - call within a thread, internally we could have a loop before exiting
     * but this version simply allocates and exits.
     */
    virtual void run()
    {
        auto alloc_func = [&]( PortInfo &a,
                               PortInfo &b,
                               void *data )
        {
            /** same alloc for all, inherit from base alloc **/
            (this)->allocate( a, b, data );
        };
        auto &container( (this)->source_kernels.acquire() );
        GraphTools::BFS( container, alloc_func );
        (this)->source_kernels.release();
        (this)->setReady();
        return;
    }
};
#endif /* END RAFTSTDALLOC_HPP */
