/**
 * map.hpp - 
 * @author: Jonathan Beard
 * @version: Fri Sep 12 10:28:33 2014
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
#ifndef _MAP_HPP_
#define _MAP_HPP_  1
#include <typeinfo>
#include <cassert>

#include "kernel.hpp"

namespace order
{
   enum spec = { in, out };
}

class Map
{
public:
   Map()          = default;
   virtual ~Map() = default;

   template < order::spec t = order::in >
      link( Kernel &a, Kernel &b )
   {
      /** assume each only has a single input / output **/
      switch( t )
      {
         case( order::in ):
         {
            /** for the moment lets assume everything is threaded **/
            auto func = a[ "" ]
         }
         break;
         case( order::out ):
         {
            assert( false );
         }
         break;
      }
   }
   
   template < order::spec t = order::in > 
      link( Kernel &a, const order::spec a_port, Kernel &b )
   {

   }
   
   template < order::spec t = order::in >
      link( Kernel &a, Kernel &b, const order::spec b_port )
   {

   }
   
   template < order::spec t = order::in >
      link( Kernel &a,const order::spec a_port, Kernel &b, const std::string b_port )
   {

   }
};
#endif /* END _MAP_HPP_ */
