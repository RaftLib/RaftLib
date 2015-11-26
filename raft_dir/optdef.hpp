/**
 * optdef.hpp - 
 * @author: Jonathan Beard
 * @version: Tue Dec  9 07:02:50 2014
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
#ifndef _OPTDEF_HPP_
#define _OPTDEF_HPP_  1

#define likely( x ) __builtin_expect( !!(x), 1 )
#define unlikely( x ) __builtin_expect( !!(x), 0 )

#endif /* END _OPTDEF_HPP_ */
