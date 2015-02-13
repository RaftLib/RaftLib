/**
 * kerneliterator.hpp - 
 * @author: Jonathan Beard
 * @version: Fri Feb 13 09:03:40 2015
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
#ifndef _KERNELITERATOR_HPP_
#define _KERNELITERATOR_HPP_  1
#include <iterator>
#include <mutex>
#include <vector>
#include "kernelcontainer.hpp"
namespace raft
{
   class kernel;
}


class KernelIterator : public std::iterator< std::forward_iterator_tag, raft::kernel >
{
private:
   KernelContainer &container;
   /** declare local iterator of the type inside the container **/
   decltype( container.list.begin() ) iterator;   
   const bool is_begin;
public:
   KernelIterator( KernelContainer &container, const bool is_begin );

   virtual ~KernelIterator();

   KernelIterator& operator++();

   /** does the obvious operation **/
   bool operator ==( const KernelIterator &rhs );
   /** does the obvious operation **/
   bool operator !=( const KernelIterator &rhs );
   /**
    * operator* - returns the current position
    */
   raft::kernel& operator*();
};

#endif /* END _KERNELITERATOR_HPP_ */
