/**
 * parsedefs.hpp - definitions for streamparse and parsemap. these basically
 * enable passing state from one parse reduction to the next.
 * @author: Jonathan Beard
 * @version: Sun Mar 19 05:12:51 2017
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
#ifndef _PARSEDEFS_HPP_
#define _PARSEDEFS_HPP_  1
#include <cstdint>
#include <array>
#include "defs.hpp"
#include "raftmanip.hpp"

namespace raft
{
namespace parse
{

enum split_state : std::int8_t { split = 0, join, cont };
    
struct manip_option
{    
    std::uint64_t  int_o    = static_cast< std::uint64_t >( 0 );
    double         float_o  = static_cast< double >( 0.0 );
};
     
struct state
{
    split_state         s               = raft::parse::cont;
    raft::order::spec   order           = raft::order::in;
    /** valid bits for array below **/
    raft::manip_vec_t   manip_vec       = raft::manip< 
                                            raft::parallel::system, 
                                            raft::vm::standard >::value;
    /** programmer should know based on vector above the type needed **/
    std::array< manip_option, sizeof( raft::manip_vec_t ) * raft::bits_per_byte >   
                        manip_options;
};

}
} /** end namespace raft **/
#endif /* END _PARSEDEFS_HPP_ */
