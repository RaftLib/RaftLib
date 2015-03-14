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
   list.insert( kernel );
   return;
}

bool
KernelContainer::removeKernel( raft::kernel *kernel )
{
   assert( kernel != nullptr );
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

auto
KernelContainer::begin() -> KernelIterator< decltype( list.begin() ) >
{
   return( KernelIterator< decltype( list.begin() ) >( list.begin() ) );
}

auto
KernelContainer::end() -> KernelIterator< decltype( list.end() ) >
{
   return( KernelIterator< decltype( list.end() ) >( list.end() ) );
}

void
KernelContainer::lock()
{
   while( access.try_lock() == false )
   {
      //std::this_thread::yield();
   }
   return;
}

void
KernelContainer::unlock()
{
   access.unlock();
   return;
}
