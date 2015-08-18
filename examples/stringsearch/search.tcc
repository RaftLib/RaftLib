/**
 * search.tcc - 
 * @author: Jonathan Beard
 * @version: Sun Nov  2 06:23:18 2014
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
#ifndef _SEARCH_TCC_
#define _SEARCH_TCC_  1
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cmath>

#include <raft>
#include <raftio>
#include "searchdefs.hpp"
#include "rkverifymatch.hpp"

namespace raft
{
   enum searchalgorithm { rabinkarp, 
                          ahocorasick,
                          boyermoore,
                          kmp };

template < searchalgorithm alg > class search;

#include "rksearch.tcc"
#include "acsearch.tcc"
#include "bmsearch.tcc"

} /** end namespace raft **/
#endif /* END _SEARCH_TCC_ */
