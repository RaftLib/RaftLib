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
#include <cassert>
#include <mutex>


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
   lock();
   list.insert( kernel );
   unlock();
}

bool
KernelContainer::removeKernel( raft::kernel *kernel )
{
   assert( kernel != nullptr );
   /** get both locks **/
   lock();
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
   unlock();
}

auto
KernelContainer::size() -> decltype( list.size() )
{
   return( list.size() );
}

auto
KernelContainer::begin() -> KernelIterator< decltype( list.begin() ) >
{
   lock();
   return( KernelIterator< decltype( list.begin() ) >( list.begin(), 
                                                       m_begin,
                                                       m_end ) );
}

auto
KernelContainer::end() -> KernelIterator< decltype( list.end() ) >
{
   return( KernelIterator< decltype( list.end() ) >( list.end(), 
                                                     m_begin,
                                                     m_end ) );
}

void
KernelContainer::lock()
{
   while( std::try_lock( m_begin, m_end ) != true )
   {
      m_begin.unlock();
      m_end.unlock();
   }
}

void
KernelContainer::unlock()
{
   m_begin.unlock();
   m_end.unlock();
}
