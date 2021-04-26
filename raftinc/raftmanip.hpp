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

namespace raft
{

template < class...  MODIFIERS > class manip
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
    constexpr template < class... KERNELS > static void bind( KERNELS&&... kernels )
    {
       
        return;
    }


private:
#if 0
    template < manip_vec_t value, class... KERNELS > struct bind_helper{};
    
    /** 
     * struct that doesn't do anything, just catch the end
     * condition for the recursion 
     */
    template < manip_vec_t value > struct bind_helper< value >
    {
        constexpr static void bind()
        {
            return;
        }
    };
    
    
    /** END HELPERS TO BIND **/
    template < manip_vec_t value, class KERNEL, class... KERNELS > 
        struct bind_helper< value, KERNEL, KERNELS... >
    {
        static void bind( KERNEL &&kernel, KERNELS&&... kernels )
        {
            /** recursively call for each kernel **/
            manip_local::apply_help( value, kernel );
            bind_helper< value, KERNELS... >::bind( 
                std::forward< KERNELS >( kernels )... );
            return;
        }
    };
#endif    
    /**
     * we don't need these given we don't want anybody 
     * to actually instantiate one of these. 
     */
    manip() = delete;
    ~manip() = delete;

};

} /** end namespace raft **/ 

#endif /* END MANIPBASE_HPP */
