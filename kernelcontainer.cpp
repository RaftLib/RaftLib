/**
 * kernelcontainer.cpp - 
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
#include "kernelcontainer.hpp"
#include "kerneliterator.hpp"
#include <cassert>

KernelContainer::KernelContainer()
{
}

KernelContainer::~KernelContainer()
{
}

void
KernelContainer::addKernel( raft::kernel *kernel )
{
   assert( kernel != nullptr );
   std::lock_guard< decltype( m_begin ) > lb( m_begin );
   std::lock_guard< decltype( m_end ) >   le( m_end );
   list.insert( kernel );
}

bool
KernelContainer::removeKernel( raft::kernel *kernel )
{
   assert( kernel != nullptr );
   /** get both locks **/
   std::lock_guard< decltype( m_begin ) > lb( m_begin );
   std::lock_guard< decltype( m_end ) >   le( m_end );
   auto el( list.find( kernel ) );
   if( el == list.end() )
   {
      return( false );
   }
   else
   {
      list.erase( el );
      return( true );
   }
}

auto
KernelContainer::size() -> decltype( list.size() )
{
   return( list.size() );
}

KernelIterator
KernelContainer::begin()
{
   return( KernelIterator( (*this), true ) );
}

KernelIterator
KernelContainer::end()
{
   return( KernelIterator( (*this), false ) );
}
