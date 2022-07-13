/**
 * roundrobin.hpp -
 * @author: Jonathan Beard
 * @version: Tue Oct 28 13:05:38 2014
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
#ifndef RAFTROUNDROBIN_HPP
#define RAFTROUNDROBIN_HPP  1

#include "fifo.hpp"
#include "port.hpp"
#include "splitmethod.hpp"

class roundrobin : public splitmethod
{
public:
    roundrobin() : splitmethod()
    {
    }
    virtual ~roundrobin()
    {
    }

protected:
    virtual FIFO* select_fifo( Port &port_list, const functype type )
    {
        for( ;; )
        {
            for( auto &port : port_list )
            {
                /**
                 * TODO, big assumption here is that
                 * eventually a port will have space
                 */
                if( type == sendtype && port.space_avail() > 0 )
                {
                    return( &( port ) );
                }
                if( type == gettype && port.size() > 0 )
                {
                    return( &( port ) );
                }
            }
        }
    }
};
#endif /* END RAFTROUNDROBIN_HPP */
