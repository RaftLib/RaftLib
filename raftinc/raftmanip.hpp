/**
 * manip.hpp - 
 * @author: Jonathan Beard
 * @version: Wed Apr  7 06:40:24 2021
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
#ifndef MANIPBASE_HPP
#define MANIPBASE_HPP  1
#include "kernel.hpp"
#include "port_info.hpp"
#include "defs.hpp"
#include <cstddef>
#include <functional>
#include <array>


namespace raft
{

//base case
template < class KERNELS, class... MODS  > struct manip_helper{};

//end recursion
template < class KERNELS > struct manip_helper< KERNELS >
{
    constexpr static void bind_helper( KERNELS&& kernel_list )
    {
        UNUSED( kernel_list );
        return;
    }
};

//main worker
template < class KERNELS, class MOD, class... MODS > struct 
    manip_helper< KERNELS, MOD, MODS... >
{
    using manip_t = manip_helper< KERNELS, MODS... >;
    constexpr static void bind_helper( KERNELS&& kernel_list )
    {
    
        for( auto &p : kernel_list )
        {
            MOD::invoke( std::forward< raft::kernel >( p.get() ) );
        }
        manip_t::bind_helper( std::forward< KERNELS >( kernel_list ) );
    }
};

template < class... MODS > class manip
{
public:

    /**
     * bind - right now this returns void, needs to return
     * a variable size constant array of the inputs...would
     * be more convenient at least. 
     * bind itself takes each struct input parameter class and 
     * calls the static constexpr invoke function of those 
     * functions on the calling kernels. 
     * @param - variable count of raft::kernel derived objects. 
     * @return - void for now, see notes at top.
     */
    template < class... KERNELS > constexpr static auto bind( KERNELS&&... kernels )
    {
        //unkernel_list
        const auto psize = sizeof...(kernels);
        using kernel_ref_t   = std::reference_wrapper< raft::kernel >;
        using param_kernel_t = std::array< kernel_ref_t, psize >;
        param_kernel_t param_kernels  = { kernels... };
        
        //apply to each
        raft::manip_helper< param_kernel_t, MODS... >::bind_helper( 
                                   std::forward< param_kernel_t >( param_kernels) );
        return( param_kernels );                               
    }
};

} /** end namespace raft **/ 

#endif /* END MANIPBASE_HPP */
