/**
 * internaldefs.hpp - 
 * @author: Jonathan Beard
 * @version: Mon Apr  3 04:49:41 2017
 * 
 * Copyright 2017 Jonathan Beard
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
#ifndef RAFTINTERNALDEFS_HPP
#define RAFTINTERNALDEFS_HPP  1

#ifdef _MSC_VER
#    if (_MSC_VER >= 1800)
#        define __alignas_is_defined 1
#    endif
#    if (_MSC_VER >= 1900)
#        define __alignof_is_defined 1
#    endif
#else 
#    ifndef __APPLE__
#    include <cstdalign>   // __alignas/of_is_defined directly from the implementation
#    endif
#endif

/**
 * should be included, but just in case there are some 
 * compilers with only experimental C++11 support still
 * running around, check macro..turns out it's #ifdef out 
 * on GNU G++ so checking __cplusplus flag as indicated
 * by https://goo.gl/JD4Gng
 */
#if ( __alignas_is_defined == 1 ) || ( __cplusplus >= 201103L )
#    define ALIGN(X) alignas(X)
#else
#    pragma message("C++11 alignas unsupported :( Falling back to compiler attributes")
#    ifdef __GNUG__
#        define ALIGN(X) __attribute__ ((aligned(X)))
#    elif defined(_MSC_VER)
#        define ALIGN(X) __declspec(align(X))
#    else
#        error Unknown compiler, unknown alignment attribute!
#    endif
#endif

#if ( __alignas_is_defined == 1 ) || ( __cplusplus >= 201103L )
#    define ALIGNOF(X) alignof(x)
#else
#    pragma message("C++11 alignof unsupported :( Falling back to compiler attributes")
#    ifdef __GNUG__
#        define ALIGNOF(X) __alignof__ (X)
#    elif defined(_MSC_VER)
#        define ALIGNOF(X) __alignof(X)
#    else
#        error Unknown compiler, unknown alignment attribute!
#    endif
#endif


#if (defined __linux) || (defined __APPLE__ )

#define R_LIKELY( var ) __builtin_expect( var, 1 )
#define R_UNLIKELY( var ) __builtin_expect( var, 0 )

#else

#define R_LIKELY( var ) var
#define R_UNLIKELY( var ) var

#endif

#endif /* END RAFTINTERNALDEFS_HPP */
