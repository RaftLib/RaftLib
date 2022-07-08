/**
 * kernel_wrapper.hpp - this class is designed to be used only by the kernel.hpp
 * make kernel function. It's supposed to be a wrapper class for the kernel type
 * and sub-types. It should ahve a destructor that destroys the wrapped pointer
 * on scope exit if it hasn't already been deallocated (attempting to be extra
 * safe). Its main function is to provide a syntax marker for the operator >>
 * overloads in the kpair.hpp file so that the compiler knows exactly what to
 * link to. The other reason for this class's existence is b/c we can't really
 * use the move syntax without a complicated copy. The kernel itself cannot live
 * on the stack, it must live in the heap or elsewhere.
 *
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
#ifndef RAFTKERNEL_WRAPPER_HPP
#define RAFTKERNEL_WRAPPER_HPP  1
#include <cstdint>
#include "kernel.hpp"

/** pre-declare kpair, header included in cpp file **/
class kpair;

namespace raft
{


class kernel_wrapper
{
    const static std::uintptr_t sentinel = 0x7070707070707070;
public:
    /**
     * kernel_wrapper - copy constructor. Takes ownership
     * of the kernel wrapped by other, sets other to the
     * sentinel value so that it no longer has a valid kernel
     * to own, and it also prevents a double free.
     * @param   other - const kernel_wrapper
     */
    kernel_wrapper( const kernel_wrapper &other ) : k( other.k )
    {
        /**
         * ensures other wrapper no longer valid, also prevents
         * double free.
         */
        const_cast<kernel_wrapper&>( other ).k
            = reinterpret_cast< raft::kernel* >( kernel_wrapper::sentinel );
    }

    /**
     * destructor deletes stored kernel
     * if it exits scope...should exit
     * scope if it's actually being used
     */
    virtual ~kernel_wrapper()
    {
        /**
         * check for sentinal value, if it's there..then we don't need
         * to delete this one as the map object already has ownership
         */
        if( reinterpret_cast< std::uintptr_t >( k ) !=
            kernel_wrapper::sentinel )
        {
            delete( k );
            /**
             * set sentinal value just in case this one happens to belong to
             * two wrapper objects. It also helps to make sure you can't
             * accidentally pass the map an invalid wrapped kernel
             */
            k = reinterpret_cast< raft::kernel* >( kernel_wrapper::sentinel );
        }
    }

    template < class T /** kernel type **/,
               class ... Args >
    static kernel_wrapper make( Args&&... params )
    {
        auto *k( new T( std::forward< Args >( params )... ) );
        k->internal_alloc = true;
        return( kernel_wrapper( k ) );
    }


protected:
    /**
     * kernel_wrapper - constructor for the wrapper
     * object, this does exactly what it says and
     * sets the wrapper to the parameter kernel.
     * @param   k   - raft::kernel* const
     */
    kernel_wrapper( raft::kernel * const k ) : k( k ){};

    /**
     * operator * - this function will be used by the
     * kpair object when it takes ownership of the
     * pointer. This will then be passed to the map
     * object. The wrapped pointer will be set with
     * the sentinal value so that it isn't double
     * freed.
     * @return raft::kernel*
     */
    raft::kernel* operator * ()
    {
        auto * const ptr( k );
        /** reset sentinel value so that the delete doesn't accidentally
         * delete it **/
        k = reinterpret_cast< raft::kernel* >( kernel_wrapper::sentinel );
        return( ptr );
    }


    raft::kernel *k = reinterpret_cast< raft::kernel* >( sentinel );

    /**
     * kernel will need to access the constructor for the
     * static make function
     */
    friend class kernel;
    /**
     * kpair will need to access so it can get to the operator*
     * overload
     */
    friend class ::kpair;
};

}

#endif /* END RAFTKERNEL_WRAPPER_HPP */
