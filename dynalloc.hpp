/**
 * dynalloc.hpp - 
 * @author: Jonathan Beard
 * @version: Mon Oct 13 16:36:18 2014
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
#ifndef _DYNALLOC_HPP_
#define _DYNALLOC_HPP_  1
#include "allocate.hpp"

class Map;
class dynalloc : public Allocate
{
public:
   dynalloc( Map &map );

   virtual ~dynalloc();

   virtual void run();

private:
   static std::size_t hash( PortInfo &a, PortInfo &b );
};

#endif /* END _DYNALLOC_HPP_ */
