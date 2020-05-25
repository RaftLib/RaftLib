/**
 * fifoabstract.tcc - encapsulate some of the common components
 * of a FIFO implementation.  
 *
 * @author: Jonathan Beard
 * @version: Sun Sep  7 20:36:12 2014
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
#ifndef FIFOABSTRACT_TCC
#define FIFOABSTRACT_TCC  1
#include "ringbuffertypes.hpp"
#include "bufferdata.tcc"
#include "blocked.hpp"
#include "fifo.hpp"


template < class T, Type::RingBufferType type > 
   class FIFOAbstract : public FIFO
{
public:
   FIFOAbstract() : FIFO()
   {
   }

protected:
   /** TODO, package this as as struct **/
   volatile bool            allocate_called = false;
   Blocked::value_type      n_allocated     = 1;
};
#endif /* END FIFOABSTRACT_TCC */
