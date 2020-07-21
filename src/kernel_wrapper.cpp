/**
 * kernel_wrapper.cpp - 
 * @author: Jonathan Beard
 * @version: Thu Sep  1 16:12:43 2016
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
#include "kernel_wrapper.hpp"
#include "kernel.hpp"

using namespace raft;

kernel_wrapper::kernel_wrapper( raft::kernel * const k ) : k( k ){}

kernel_wrapper::kernel_wrapper( const kernel_wrapper &other ) : k( other.k )
{
    /** 
     * ensures other wrapper no longer valid, also prevents
     * double free.
     */
    const_cast<kernel_wrapper&>( other ).k 
        = reinterpret_cast< raft::kernel* >( kernel_wrapper::sentinel ); 
}

kernel_wrapper::~kernel_wrapper()
{
    /** 
     * check for sentinal value, if it's there..then we don't need
     * to delete this one as the map object already has ownership
     */
    if( reinterpret_cast< std::uintptr_t >( k ) != kernel_wrapper::sentinel )
    {
        delete( k );
        /** 
         * set sentinal value just in case this one happens to belong to two
         * wrapper objects. It also helps to make sure you can't accidentally
         * pass the map an invalid wrapped kernel
         */
        k = reinterpret_cast< raft::kernel* >( kernel_wrapper::sentinel );
    }
}

raft::kernel* kernel_wrapper::operator *()


{
    auto * const ptr( k );
    /** reset sentinel value so that the delete doesn't accidentally delete it **/
    k = reinterpret_cast< raft::kernel* >( kernel_wrapper::sentinel );
    return( ptr );
}

raft::kernel&
kernel_wrapper::operator []( const std::string &&portname )
{
   if( k->enabled_port.size() < 2 )
   {
        k->enabled_port.push( portname );
   }
   else
   {
        throw AmbiguousPortAssignmentException(
            "too many ports added with: " + portname
        );
   }
   return( (*k) );
}
