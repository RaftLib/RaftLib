/**
 * kernel.hpp - 
 * @author: Jonathan Beard
 * @version: Thu Sep 11 15:34:24 2014
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
#ifndef _KERNEL_HPP_
#define _KERNEL_HPP_  1
#include "port.hpp"

enum dir = { input, output };

class Kernel
{
public:
   Kernel()         = default;
   virtual Kernel() = default;


   virtual bool run() = 0;

protected:
   Port               input;
   Port               output;
   
   friend class Map;
};
#endif /* END _KERNEL_HPP_ */
