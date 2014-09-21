/**
 * stdalloc.hpp - 
 * @author: Jonathan Beard
 * @version: Sat Sep 20 19:56:49 2014
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
#ifndef _STDALLOC_HPP_
#define _STDALLOC_HPP_  1
#include "map.hpp"
#include "allocate.hpp"

class stdalloc : public Allocate
{
public:
   stdalloc( Map &map );
   virtual ~stdalloc();

   virtual void run();
};
#endif /* END _STDALLOC_HPP_ */
