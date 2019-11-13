/**
 * parsetreedefs.hpp - 
 * @author: Jonathan Beard
 * @version: Mon Nov 11 12:11:48 2019
 * 
 * Copyright 2019 Jonathan Beard
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
#ifndef _PARSETREEDEFS_HPP_
#define _PARSETREEDEFS_HPP_  1
#include <vector>
#include "kernel.hpp"
#include <stack>
#include <memory>

using group_t       = std::vector< raft::kernel* >;
using group_ptr_t   = std::unique_ptr< group_t >;
using frontier_t    = std::unique_ptr< group_ptr_t >;

#endif /* END _PARSETREEDEFS_HPP_ */
