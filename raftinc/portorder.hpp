/**
 * portorder.hpp - 
 * @author: Jonathan Beard
 * @version: Wed Mar 16 13:11:49 2016
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
#ifndef RAFTPORTORDER_HPP
#define RAFTPORTORDER_HPP  1
#include <cstdint>

namespace raft
{

namespace order
{
    enum spec : std::uint8_t { in = 0, out = 1 };
}

} /** end namespace raft **/
#endif /* END RAFTPORTORDER_HPP */
