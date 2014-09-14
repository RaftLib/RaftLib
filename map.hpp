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
class Map
{
public:
   Map()          = default;
   virtual ~Map() = default;

   template < std::string t >
      link( Kernel &a, Kernel &b )
   {
      /** assume each only has a single input / output **/
      const auto order( order_map[ t ] );
      switch( order )
      {
         case( INORDER ):
         {
            a[ "" ]
         }
         break;
         case( OUTORDER ):
         {
            assert( false );
         }
         break;
      }
   }
   
   template < std::string t >
      link( Kernel &a, const std::string a_port, Kernel &b )
   {

   }
   
   template < std::string t >
      link( Kernel &a, Kernel &b, const std::string b_port )
   {

   }
   
   template < std::string t >
      link( Kernel &a,const std::string a_port, Kernel &b, const std::string b_port )
   {

   }
private:
   enum    order = { INORDER, OUTORDER };
   const   std::map< std::string, order > order_map = { { "inorder", INORDER },
                                                        { "outorder", OUTORDER } };
};
#endif /* END _MAP_HPP_ */
