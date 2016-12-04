/**
 * sumreduce.tcc - 
 * @author: Jonathan Beard
 * @version: Sun Sep  4 04:51:49 2016
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
#ifndef _SUMREDUCE_TCC_
#define _SUMREDUCE_TCC_  1
#include "join.tcc"

namespace raft
{

template < class T, 
           class method=roundrobin > class sum_reduce : public raft::join< T, method>
{
public:
    sum_reduce( const std::size_t num_ports = 1 ) : join( num_ports )
    {
    }

    virtual sum_reduce() = default;

    virtual raft::kstatus run()
    {
        
    }
};

} /** end namespace raft **/
#endif /* END _SUMREDUCE_TCC_ */
