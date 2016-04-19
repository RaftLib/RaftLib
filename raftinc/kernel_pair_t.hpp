/**
 * kernel_pair_t.hpp - 
 * @author: Jonathan Beard
 * @version: Mon Apr 18 20:39:53 2016
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
#ifndef _KERNEL_PAIR_T_HPP_
#define _KERNEL_PAIR_T_HPP_  1
#include <functional>
#include <vector>

/** pre-declare some stuff **/
namespace raft
{
    class kernel;
}

class kernel_pair_t
{
public:
   constexpr kernel_pair_t( raft::kernel *a, 
                            raft::kernel *b ) : 
                                           src( a ),
                                           dst( b )
   {
   }

   constexpr kernel_pair_t( const kernel_pair_t &other ) : src( other.src ),
                                                 dst( other.dst )
   {
   }
   
   kernel_pair_t& operator = ( const kernel_pair_t &other )
   {
      src = other.src;
      dst = other.dst;
      return( *this );
   }

   raft::kernel& getSrc() noexcept 
   {
      return( *src );
   }

   raft::kernel& getDst() noexcept
   {
      return( *dst );
   }


private:
   raft::kernel *src = nullptr;
   raft::kernel *dst = nullptr;
};

#endif /* END _KERNEL_PAIR_T_HPP_ */
