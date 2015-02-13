/**
 * kerneliterator.cpp - 
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
#include "kerneliterator.hpp"
#include "kernelcontainer.hpp"

KernelIterator::KernelIterator( KernelContainer &container, const bool is_begin ) : container( container ),
                                                                                    is_begin( is_begin )
{
   if( is_begin ) 
   {
      container.m_begin.lock();
      iterator = container.list.begin();
   }
   else
   {
      container.m_end.lock();
      iterator = container.list.end();
   }
}

KernelIterator::~KernelIterator()
{
   if( is_begin )
   {
      container.m_begin.unlock();
   }
   else
   {
      container.m_end.unlock();
   }
}

KernelIterator& KernelIterator::operator++()
{
   ++iterator;
   return( (*this) );
}

bool
KernelIterator::operator==( const KernelIterator &rhs )
{
   return( iterator == rhs.iterator );
}

bool
KernelIterator::operator !=( const KernelIterator &rhs )
{
   return( iterator != rhs.iterator );
}

raft::kernel&
KernelIterator::operator *()
{
   return( *(*iterator) );
}
