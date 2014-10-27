/**
 * writecontainer.tcc - 
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
#ifndef _WRITECONTAINER_TCC_
#define _WRITECONTAINER_TCC_  1
#include <iterator>

/** 
 * TODO: add functor as an option to set signal handlers
 * so the write object can respond to data stream or 
 * asynch signals appropriately
 */
template < class T, 
           class Position > class write_container : public parallel_k
{
public:
   write_container( Position &position_it ) : p( position_it )
   {
      input.addPort< T >( "0" );
      /* no output ports */
   }

   virtual raft::kstatus run()
   {
      for( auto &port : input )
      {
         if( port.size() > 0 )
         {
            port.pop( (*p) );
            /** hope the iterator defined overloaded ++ **/
            p++;
         }
      }
      return( raft::proceed );
   }

private:
   Position  &p;
};
#endif /* END _WRITECONTAINER_TCC_ */
