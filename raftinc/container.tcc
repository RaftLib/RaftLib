/**
 * container.tcc - 
 * @author: Jonathan Beard
 * @version: Sat Jan 24 14:53:50 2015
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
#ifndef _CONTAINER_TCC_
#define _CONTAINER_TCC_  1
#include <vector>
#include <mutex>
#include <cassert>

template < class T > container
{
public:
   container() = default;
   virtual ~conainer() = default;

   void add( T * const ptr )
   {
      assert( ptr != nullptr );
      mutex.lock();
      list.push_back( ptr );
      mutex.unlock();
   }

   template < std::function< bool ( T* ) > FUNCTION >
      void remove()
   {
      mutex.lock();
      for( auto it( list.begin() ); it != list.end(); ++it )
      {
         if( FUNCTION( (*it) ) )
         {
            /** reset list to next item while removing, keep going **/
            it = list.remove( it );
         }
      }
      mutex.unlock();
      return;
   }

   virtual ItemIterator begin();
   virtual ItemIterator end();

private:
   std::vector< T* > list;
   std::mutex        mutex;
};


#endif /* END _CONTAINER_TCC_ */
