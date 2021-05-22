/**
 * kernelmanip.tcc - 
 * @author: Jonathan Beard
 * @version: Tue Apr  6 06:38:06 2021
 * 
 * Copyright 2021 Jonathan Beard
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
#ifndef KERNELMANIP_TCC
#define KERNELMANIP_TCC  1
#include "defs.hpp"
#include "kernel.hpp"

namespace raft
{

namespace parallel
{
using _size_t = std::size_t;

template < _size_t N > struct affinity_group
{
    
    constexpr static core_id_t value = N;
    
    constexpr static void invoke( raft::kernel &&k )
    {
        k.setAffinityGroup(  value );
    }

};

/**
 * right now, just for CPUs. Would be great if 
 * we could have an interface for each, but, there's
 * not a good way to do static function interfaces. 
 */
enum device_t { cpu };

template < device_t D, _size_t N > struct device
{
    constexpr static core_id_t value = N;
    
    constexpr static void invoke( raft::kernel &&k )
    {
        k.setCore( N );
    }
    //FIXME, need to add constexpr code for GPU kernel and 
    //devices on the kernel side, so, invoke would select
    //statically, but leave open dynamic migration. 
};

} /** end namespace vm **/

} /** end namespace raft **/
#endif /* END KERNELMANIP_TCC */
