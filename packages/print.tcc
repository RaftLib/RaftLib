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
#include <ostream>
#include <iostream>
#include <raft>
#include <cstdlib>

namespace raft{

class printbase
{
protected:
   std::ostream *ofs = nullptr;
};


template< typename T > class printabstract : public raft::kernel, 
                                             public raft::printbase
{
public:
   printabstract( ) : raft::kernel(),
                      raft::printbase()
   {
      input.addPort< T >( "in" );
      ofs = &(std::cout);
   }
   
   printabstract( std::ostream &stream ) : raft::kernel(),
                                           raft::printbase()
   {
      input.addPort< T >( "in" );
      ofs = &stream;
   }
};

template< typename T, char delim = '\0' > class print : public printabstract< T >
{
public:
   print( ) : printabstract< T >()
   {
   }
   
   print( std::ostream &stream ) : printabstract< T >( stream )
   {
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
      auto data( (this)->input[ "in" ].template pop_s< T >() );
      *((this)->ofs) << (*data) << delim;
      return( raft::proceed );
   }
};
template< typename T > class print< T, '\0' > : public printabstract< T >
{
public:
   print( ) : printabstract< T >()
   {
   }
   
   print( std::ostream &stream ) : printabstract< T >( stream )
   {
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
      auto data( (this)->input[ "in" ].template pop_s< T >() );
      *((this)->ofs) << (*data);
      return( raft::proceed );
   }
};

} /* end namespace raft */
#endif /* END _PRINT_TCC_ */
