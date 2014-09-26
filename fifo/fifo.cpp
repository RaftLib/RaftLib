/**
 * fifo.cpp - 
 * @author: Jonathan Beard
 * @version: Thu Sep  4 12:59:45 2014
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
#include "fifo.hpp"

FIFO::FIFO()
{

}

void
FIFO::get_zero_read_stats( Blocked &copy )
{
   /** default version does nothing at all **/
   return;
}

void
FIFO::get_zero_write_stats( Blocked &copy )
{
   /** default version does nothing at all **/
   return;
}
