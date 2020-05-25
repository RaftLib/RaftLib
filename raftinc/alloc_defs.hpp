/**
 * alloc_defs.hpp - a place to put internal allocation definitions
 * for use in deciding what type of memory to allocate within the
 * allocator for a particular edge in the graph. Right now there
 * are only a few choices, but that could likely change.
 *
 * @author: Jonathan Beard
 * @version: Tue Aug 16 19:47:01 2016
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
#ifndef RAFTALLOC_DEFS_HPP
#define RAFTALLOC_DEFS_HPP  1

enum memory_type : std::uint8_t { heap, SHM };

#endif /* END RAFTALLOC_DEFS_HPP */
