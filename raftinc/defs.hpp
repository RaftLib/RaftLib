/**
 * defs.hpp - 
 * @author: Jonathan Beard
 * @version: Sun Feb  7 05:46:48 2016
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
#ifndef _DEFS_HPP_
#define _DEFS_HPP_  1
#include <set>
#include <map>
#include <functional>
#include <type_traits>
#include <cstdint>

template < typename T > 
    using set_t = std::set< T >;

using ptr_set_t = set_t< std::uintptr_t >;
using recyclefunc_t = std::function< void( void * ) >;
using ptr_map_t = std::map< std::uintptr_t, recyclefunc_t >;
using ptr_t = std::uintptr_t;

#ifndef UNUSED 
#define UNUSED( x )[&x]{}()
#endif

#endif /* END _DEFS_HPP_ */
