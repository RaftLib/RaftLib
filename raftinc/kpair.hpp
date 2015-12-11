/**
 * kpair.hpp - 
 * @author: Jonathan Beard
 * @version: Wed Dec  9 11:36:08 2015
 * 
 * Copyright 2015 Jonathan Beard
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
#ifndef _KPAIR_HPP_
#define _KPAIR_HPP_  1

#include <string>

namespace raft
{
    class kernel;
}

class kpair
{
public:
    kpair( raft::kernel &a, raft::kernel &b );

protected:
    kpair        *next = nullptr;
private:
    raft::kernel *src;
    std::string   src_name;
    raft::kernel *dst;
    std::string   dst_name;
};

static kpair operator << ( raft::kernel &&a, raft::kernel &&b );

#endif /* END _KPAIR_HPP_ */
