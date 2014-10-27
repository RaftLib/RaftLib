/**
 * readcontainer.tcc - 
 * @author: Jonathan Beard
 * @version: Sun Oct 26 15:51:46 2014
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
#ifndef _READCONTAINER_TCC_
#define _READCONTAINER_TCC_  1
#include <raft>

template < class T,  class iterator_type > class read_container : 
   public parallel_k
{
public:
   read_container( iterator_type begin, iterator_type end ) : begin( begin ),
                                                              end( end )
   {
      output.addPort< T& >( "0" );
   }

   virtual raft::kstatus run()
   {
      for( auto &port : output )
      {
         if( begin == end )
         {
            return( raft::stop );
         }
         port.push( (*begin) );
         begin++;
      }
      return( raft::proceed );
   }

private:
   iterator_type begin;
   iterator_type end;
};
#endif /* END _READCONTAINER_TCC_ */
