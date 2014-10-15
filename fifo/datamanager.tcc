/**
 * datamanager.tcc - 
 * @author: Jonathan Beard
 * @version: Tue Oct 14 14:15:00 2014
 * 
 * Copyright 2014 Jonathan Beard
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
#ifndef _DATAMANAGER_TCC_
#define _DATAMANAGER_TCC_  1
#include <cassert>

#include "ringbuffertypes.hpp"
#include "bufferdata.tcc"

namespace Buffer
{
   enum mode { read, write };
}

template < class T, 
           Type::RingBufferType B,
           size_t SIZE = 0 > class DataManager
{
public:
   
   DataManager( ) = default;

   
   void set( Buffer::Data< T, B > *buffer )
   {
      assert( buffer != nullptr );
      buffer_a = buffer;
      buffer_b = buffer;
   }

   void resize( Buffer::Data< T, B > *new_buffer )
   {
      /** keep anyone from getting the old buffer for a second **/
      buffer_a = 0;
      auto *local_copy( buffer_b );
      const auto size( local_copy->size() );
      if( size != 0 )
      {
         T *range = ( T* ) malloc( sizeof( T ) * size );
         raft::signal signal;
         local_copy-> template pop_range< T >( range,
                                     size,
                                     signal );
         /** TODO: gotta fix the insert so it'll work with raw pointers too **/
         for( std::size_t index( 0 ); index < size; index++ )
         {
            if( index == size - 1 )
            {
               new_buffer->push( range[ index ] );
            }
            else
            {
               new_buffer->push( range[ index ], signal );
            }
         }
         free( range );
      }
      delete( local_copy );
      (this)->set( new_buffer );
   }

   Buffer::Data< T, B >* get( Buffer::mode m = read )
   {
      struct Copy
      {
         Copy( Buffer::Data< T, B > *a, Buffer::Data< T, B > *b ) : a( a ), b( b )
         {
         }

         Buffer::Data< T, B > *a;        
         Buffer::Data< T, B > *b;  
      }copy( buffer_a, buffer_b );
      while( copy.a != copy.b )
      {
         copy.a = buffer_a;
         copy.b = buffer_b;
      }
      return( copy.a );
   }

private:
   Buffer::Data< T, B > *buffer_a = (Buffer::Data< T, B >*) 1; 
   Buffer::Data< T, B > *buffer_b = (Buffer::Data< T, B >*) 0;

};
#endif /* END _DATAMANAGER_TCC_ */
