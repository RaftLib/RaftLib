/**
 * kernelkeeper.tcc - contain the locking and unlocking of 
 * containers related to raft compute kernels
 * @author: Jonathan Beard
 * @version: Sat Aug 15 09:45:01 2015
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
#ifndef _KERNELKEEPER_TCC_
#define _KERNELKEEPER_TCC_  1
#include <set>
#include <cstdio>
#include <cstdlib>
#include <thread>
#include <type_traits>
#include <functional>
#include <cassert>
#include <cstring>
#include <mutex>

namespace raft
{
   class kernel;
}

template< class CONTAINER, class CONTAINERTYPE > class keeper
{
private:
   std::mutex        mutex;
   std::thread::id   owner_id;
   CONTAINER         container;

    
public:
   
   using value_type = typename std::remove_reference< decltype( container ) >::type;
   
   keeper() = default;


   virtual ~keeper() = default;

   //FIXME: this will only work for sets, add specializations for others
   //based on insert function
   void  operator += ( CONTAINERTYPE * const ele )
   {
      acquire();
      container.emplace( ele );
      release();
      return;
   }

   void unsafeAdd( CONTAINERTYPE * const ele )
   {
      container.emplace( ele );
      return;
   }

   CONTAINER& acquire()
   {
      //spin until we can get a lock
      while( ! mutex.try_lock() )
      {
         //it's polite to yield
         std::this_thread::yield();
      }
      //we have a lock, get id
      owner_id = std::this_thread::get_id();
      return( container );
   }

   void release()
   {
#ifndef NDEBUG      
      const auto caller_id( std::this_thread::get_id() );
#endif      
      assert( caller_id == owner_id );
      mutex.unlock();
      return;
   }
   
   auto size() -> decltype( container.size() )
   {
      const auto size( container.size() );
      return( size );
   }


};

using kernelkeeper   = keeper< std::set< raft::kernel* >, raft::kernel >;

#endif /* END _KERNELKEEPER_TCC_ */
