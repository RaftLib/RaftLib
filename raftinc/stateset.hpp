/**
 * stateset.hpp - 
 * @author: Jonathan Beard
 * @version: Sat Apr  8 04:57:54 2017
 * 
 * Copyright 2017 Jonathan Beard
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
#ifndef _STATESET_HPP_
#define _STATESET_HPP_  1

/** declared in port_info.hpp **/
struct PortInfo;

namespace raft
{
/** declared in kernel.hpp **/
class kernel;

namespace parse
{

class stateset
{
public:
    stateset() = default;
    virtual ~stateset() = default;

    virtual void apply( PortInfo &a, PortInfo &b );
    virtual void apply( raft::kernel * const k );
};

} /** end namespace parse **/

} /** end namespace raft **/

#endif /* END _STATESET_HPP_ */
