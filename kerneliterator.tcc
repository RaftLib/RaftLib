/**
 * kerneliterator.tcc - 
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
#ifndef _KERNELITERATOR_TCC_
#define _KERNELITERATOR_TCC_  1
#include <iterator>
#include <vector>
#include "kernelcontainer.hpp"

namespace raft
{
   class kernel;
}

/**
 * KernelIterator - template for iterating over a list of kernels
 * with a data structure that looks like the KernelContainer object
 * @param T - class for container, default is KernelContainer
 * @param L - class for desired iterator type
 */
template < class T > class KernelIterator : 
   public std::iterator< std::forward_iterator_tag, raft::kernel >
{
private:
   /** declare local iterator of the type inside the container **/
   T iterator;   
public:
   KernelIterator( T iterator ) : iterator( iterator )
   {
   }
   
   virtual ~KernelIterator()
   {
   }

   /** increment in the usual way **/ 
   KernelIterator& operator++()
   {
      ++iterator;
      return( (*this) );
   }

   /** does the obvious operation **/
   bool operator ==( const KernelIterator &rhs )
   {
      return( iterator == rhs.iterator );
   }
   /** does the obvious operation **/
   bool operator !=( const KernelIterator &rhs ) 
   {
      return( iterator != rhs.iterator );
   }
   /**
    * operator* - returns the current position
    */
   raft::kernel& operator*()
   {
      return( *(*iterator) );
   }
};

#endif /* END _KERNELITERATOR_TCC_ */
