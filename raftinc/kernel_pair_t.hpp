/**
 * kernel_pair_t.hpp - class returned by the (relatively) deprecated
 * link operators. These should only be used in the run-time...however
 * some older code may still have it exposed and running around.
 *
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
#include <utility>

/** pre-declare some stuff **/
namespace raft
{
    class kernel;
}

/**
 * basic idea here is that we want a return type
 * for the link() function calls as well as the
 * operator += overload on the map that enables
 * return of multiple compute kernels for the
 * user to stitch together in another invocation
 * to add more kernels to the map. What we also 
 * want to ensure is that there is no way for
 * the user to delete one of these kernels which
 * could create for some interesting bugs.
 * the std::pair seems like an obvious choice
 * to return, however creating a container as part
 * of the std::pair is a bit hacky so we'll use the 
 * container directly, the next consideration is what
 * type of container to use inside that std::pair
 * which should be expandable. Again, the obvious
 * choice seems like it should be the std::vector
 * which needs to hold references so they can't
 * be easily deleted..which means we'll have to 
 * use std::reference_wrapper as well.n
 */
class kernel_pair_t
{
    /** container with ref wrapper **/ 
    using kernel_pair_t_container  = 
        std::vector< std::reference_wrapper< raft::kernel> >;

public:
    /**
     * define iterator type publicly 
     */
    using kernel_iterator_type = kernel_pair_t_container::iterator;
    /** 
     * endpoint ret type is a std::pair with
     * two iterators, one for begin, the second 
     * for end 
     */
    using endpoint_ret_type    =
        std::pair< kernel_iterator_type,
                   kernel_iterator_type >;
    /**
     * define a size type that matches the container
     * type, whatever that container type may end 
     * up being
     */
    using size_type = typename kernel_pair_t_container::size_type;
    
    /**
     * kernel_pair_t - default constructor, simply
     * reserves some space for the container, assumes
     * container has a reserve(xx) function to call
     * in the first place...likely need to add a 
     * enable_if to make sure that is always  the case
     */
    kernel_pair_t();

    /**
     * kernel_pair_t - construct by first calling the 
     * base constructor, then insert into the containers
     * with the parameters of this constructor.
     * @param   src - raft;:kernel, source kernel
     * @param   dst - raft::kernel, destination kernel
     */
    kernel_pair_t( raft::kernel * const src,
                   raft::kernel * const dst );

    /**
     * kernel_pair_t - construct by first calling the 
     * base constructor, then insert into the containers
     * with the parameters of this constructor.
     * @param   src - raft;:kernel, source kernel
     * @param   dst - raft::kernel, destination kernel
     */
    kernel_pair_t( raft::kernel &src,
                   raft::kernel &dst );

    /**
     * getSrc - return a std::pair object with iterators
     * to the source and destination of the list of 
     * sources added in the last map addition. Again,
     * pair.first maps ot begin(), and pair.second maps
     * to end();
     * @return endpoint_ret_type
     */
    kernel_pair_t::endpoint_ret_type getSrc();
    /**
     * getSrcSize - returns the size of the source
     * container. This is the number of kernels
     * within the last map addition.
     * @return size_type - number of kernels in source
     *                     container
     */
    kernel_pair_t::size_type         getSrcSize() noexcept; 
    /**
     * getDst - return a std::pair object with iterators
     * to the dst list of kernels added in the last map 
     * addition. Again, pair.first maps ot begin(), and 
     * pair.second maps to end();
     * @return endpoint_ret_type
     */
    kernel_pair_t::endpoint_ret_type getDst();
    /**
     * getDstSize - returns the size of the destination
     * container. This is the number of kernels
     * within the last map addition.
     * @return size_type - number of kernels in source
     *                     container
     */
    kernel_pair_t::size_type         getDstSize() noexcept;
    /**
     * addSrc - add a source kernel to this pair object
     * which is retreivable by the getSrc. If this object
     * disappears (is deallocated) after you add it then
     * bad things might happen...don't delete them till
     * you've disposed of these objects.
     * @param   k   - raft::kernel&
     */
    void addSrc( raft::kernel &k ) noexcept;
    /**
     * addDst - add a destination kernel to this pair object
     * which is retreivable by the getDst. If this object
     * disappears (is deallocated) after you add it then
     * bad things might happen...don't delete them till
     * you've disposed of these objects.
     * @param   k   - raft::kernel&
     */
    void addDst( raft::kernel &k ) noexcept;
    /** 
     * clearSrc - does exactly what it says, clears out
     * the list of kernels, does not however destoy them
     * so they're still valid kernels, just not available
     * to the list anymore
     */
    void clearSrc() noexcept;
    /** 
     * clearDst - does exactly what it says, clears out
     * the list of kernels, does not however destoy them
     * so they're still valid kernels, just not available
     * to the list anymore
     */
    void clearDst() noexcept;

private:
    /** type is determined by using type aliases above the first public: **/
    kernel_pair_t_container source;
    kernel_pair_t_container destination;
};


#endif /* END _KERNEL_PAIR_T_HPP_ */
