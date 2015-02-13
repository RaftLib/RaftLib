/**
 * kernelcontainer.hpp - simple container with lockable iterators
 * for use with raft::kernel objects.
 *
 * @author: Jonathan Beard
 * @version: Wed Jan 14 08:12:47 2015
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
#ifndef _KERNELCONTAINER_HPP_
#define _KERNELCONTAINER_HPP_  1
#include <mutex>
#include <vector>
#include <cstddef>
#include "kernel.hpp"

class KernelIterator;

class KernelContainer
{
private:
   std::set< raft::kernel * > list;
public:
   /**
    * Contstructor for a container for raft::kernel objects
    */
   KernelContainer();
   virtual ~KernelContainer();

   /** 
    * size - return the number of kernels currently
    * assigned
    * @return std::size_t
    */
   auto size() -> decltype( list.size() );
   /** 
    * removeKernel - call to remove the kernel specified
    * as the param, which must not be null.  The function
    * first gains a lock on the list then proceeds to 
    * remove it.
    * @param   kernel - raft::kernel const *
    * @return  bool, true if removed, false otherwise
    */
   bool removeKernel( raft::kernel *kernel );
   /**
    * addKernel - add a kernel to this container.  Gains
    * a lock before committing the kernel to the data
    * structure.
    * @param   raft::kernel*, kernel to be added
    */
   void addKernel( raft::kernel *kernel );
   /**
    * begin - returns an iterator to the first kernel in the list,
    * future behavior might produce a priority type ordering, but
    * this should be handled by the iterator and its sub-classes.
    * Iterator is thread safe!
    * @return KernelIterator
    */
   KernelIterator begin();
   /** 
    * KernelIterator - returns an iterator to one past the last 
    * kernel in the list, this cannot be relied upon to be the
    * last element since the iterator does not define a -- operator,
    * so really it can just be used for detecting when all the kernels
    * have been iterated through.
    * @return KernelIterator
    */
   KernelIterator end();
private:
   std::mutex                   m_begin;
   std::mutex                   m_end;
   friend class KernelIterator;
};
#endif /* END _KERNELCONTAINER_HPP_ */
