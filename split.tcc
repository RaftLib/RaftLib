/**
 * split.tcc - 
 * @author: Jonathan Beard
 * @version: Mon Oct 20 09:49:17 2014
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
#ifndef _SPLIT_TCC_
#define _SPLIT_TCC_  1
#include <raft>

template <class T> Split : public Kernel
{
public:
   Split()
   {
      input.addPort<  T >( "0" );
      output.addPort< T >( "0" );
   }

   virtual ~Split() = default;

   virtual raft::kstatus run()
   {
      /** assume one input port, multiple output ports **/
      /** peek from input port, push to however many output ports there are **/
   }

protected:
   
};
#endif /* END _SPLIT_TCC_ */
