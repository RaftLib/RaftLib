/**
 * pointer.cpp - 
 * @author: Jonathan Beard
 * @version: Thu May 15 09:58:51 2014
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
#include "pointer.hpp"
#include <thread>

Pointer::Pointer(const size_t cap ) : a( 0 ),
                                      b( 0 ),
                                      wrap_a( 0 ),
                                      wrap_b( 0 ),
                                      max_cap( cap )
{
}

size_t 
Pointer::val( Pointer *ptr )
{
   struct{
      size_t a;
      size_t b;
   }copy;
   do{
      copy.a = ptr->a;
      copy.b = ptr->b;
   }while( copy.a ^ copy.b );
   return( copy.b );
}

size_t 
Pointer::inc( Pointer *ptr )
{
   ptr->a = ( ptr->a + 1 ) % ptr->max_cap;
   ptr->b = ( ptr->b + 1 ) % ptr->max_cap;
   if( ptr->b == 0 )
   {
      ptr->wrap_a++;
      ptr->wrap_b++;
   }
   return( ptr->b );
}

size_t 
Pointer::incBy( const size_t in, Pointer *ptr )
{
   ptr->a = ( ptr->a + in ) % ptr->max_cap;
   ptr->b = ( ptr->b + in ) % ptr->max_cap;
   if( ptr->b < in )
   {
      ptr->wrap_a++;
      ptr->wrap_b++;
   }
   return( ptr->b );
}

size_t 
Pointer::wrapIndicator( Pointer *ptr )
{
   struct{
      size_t a;
      size_t b;
   }copy;
   do{
      copy.a = ptr->wrap_a;
      copy.b = ptr->wrap_b;
   }while( copy.a ^ copy.b );
   return( copy.b );
}
