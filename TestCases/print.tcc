/**
 * print.tcc - 
 * @author: Jonathan Beard
 * @version: Tue Sep 30 10:46:39 2014
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
#ifndef _PRINT_TCC_
#define _PRINT_TCC_  1

#include <functional>
#include <iostream>
#include <raft>
#include <cstdlib>


template< typename T, char delim = '\0' > class Print : public Kernel
{
public:
   Print( ) : Kernel()
   {
      input.addPort< T >( "in" );
   }

   /** 
    * run - implemented to take a single 
    * input port, pop the itam and print it.
    * the output isn't yet synchronized so if
    * multiple things are printing to std::cout
    * then there might be issues, otherwise
    * this works well for debugging and basic 
    * output.
    * @return raft::kstatus
    */
   virtual raft::kstatus run()
   {
      auto data( input[ "in" ].template pop_s< T >() );
      if( delim != '\0' )
      {
         std::cout << (*data) << delim;
      }
      else
      {
         std::cout << (*data);
      }
      return( raft::proceed );
   }
};

#endif /* END _PRINT_TCC_ */
