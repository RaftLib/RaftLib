/**
 * fastaread.tcc - 
 * @author: Jonathan Beard
 * @version: Mon May  4 14:21:31 2015
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
#ifndef _FASTAREAD_TCC_
#define _FASTAREAD_TCC_  1

#include <cstdint>

using nuc_read_t   = std::uint64_t;
using score_t      = std::uint8_t;
using read_index_t = std::uint64_t;
using base_t       = std::uint8_t;

enum base : base_t { A = 1, C = 3, G = 7, X = 8 };

template < typename T, 
           typename std::enable_if< std::is_unsigned< T >::value >::type* = nullptr > 
            packread( T vector, const base_t base )
            {

            };

struct readchunk
{
    
};

#endif /* END _FASTAREAD_TCC_ */
