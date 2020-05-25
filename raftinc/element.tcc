/**
 * element.tcc - 
 * @author: Jonathan Beard
 * @version: Wed Dec 31 16:00:19 2014
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
#ifndef ELEMENT_TCC
#define ELEMENT_TCC  1

namespace Buffer{

template < class X > struct Element
{

    Element() = default;

    Element( const Element< X > &other )
    {
        (this)->item = other.item;
    }

    Element< X >& operator = ( X &other )
    {
       item = other;
       return( (*this) );
    }

}; /** end Element< X > **/

} /** end namespace Buffer **/
#endif /* END ELEMENT_TCC */
