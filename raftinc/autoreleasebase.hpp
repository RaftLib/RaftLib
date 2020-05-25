/**
 * autoreleasebase.hpp - 
 * @author: Jonathan Beard
 * @version: Thu Aug 27 14:24:31 2015
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
#ifndef AUTORELEASEBASE_HPP
#define AUTORELEASEBASE_HPP  1

/**
 * this is used by the autorelease objects so that index operator []
 * can return access to both the signal and the element at that index
 */
template < class T > struct autopair
{
   autopair( T &ele, Buffer::Signal &sig ) : ele( ele ),
                                             sig( sig )
   {}

   T              &ele;
   Buffer::Signal &sig;
};

class autoreleasebase 
{
public:
   autoreleasebase()          = default;
   virtual ~autoreleasebase() = default;
   /** empty class **/
};
#endif /* END AUTORELEASEBASE_HPP */
