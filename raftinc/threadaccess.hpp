/**
 * threadaccess.hpp - 
 * @author: Jonathan Beard
 * @version: Fri Sep 23 10:59:50 2016
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
#ifndef _THREADACCESS_HPP_
#define _THREADACCESS_HPP_  1
#include <cstdint>
#include "defs.hpp"

struct ThreadAccess
{
    union
    {
        std::uint64_t whole = 0;
        dm::key_t     flag[ 8 ];
    };

    byte_t    padding[ L1D_CACHE_LINE_SIZE - 8 /** padd to cache line **/ ];
};


#endif /* END _THREADACCESS_HPP_ */
