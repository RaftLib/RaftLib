/**
 * common.hpp - static helper functions of various types 
 * @author: Jonathan Beard
 * @version: Sun May 10 19:10:06 2015
 * 
 * Copyright 2015 Jonathan Beard
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
#ifndef RAFTCOMMON_HPP
#define RAFTCOMMON_HPP  1
#include <string>
#include <cstdlib>
#include <cassert>
#include <functional>
#include <typeinfo>

class common
{
public:
/**
 * __printClassName - helper function for below function,
 * basically see the more complete docs below for the delta,
 * the string passed to this function should be the name
 * of the class from either the typeinfo or typeid( xx ).name()
 * call.
 */
static std::string __printClassName( const std::string &&obj_name );

static std::string printClassNameFromStr( const std::string &&str );

/**
 * pringClassName - takes in a class reference and 
 * prints the class name using cxx-demangle.  I basically
 * got tired of typing all the error checking code over 
 * and over so here's a simplified interface for it.  
 * @param k - Class reference for which you want the class.
 * @returns std::string
 */
template < class K > static
   std::string printClassName( K &k )
{
   return( common::__printClassName( typeid( k ).name() ) );
}


/**
 * convert_arr - used to convert the forwared parameter pack 
 * to something that's non const 
 */
template < class... F > 
constexpr static auto convert_arr( F&&... t )
#ifdef STRING_NAMES
    -> std::array< std::string, sizeof...(F) >
#else
    -> std::array< 
        std::reference_wrapper< F >, 
        sizeof...(F)
        >
#endif        
{
    return { std::forward< F >( t )... };
}


};

#endif /* END RAFTCOMMON_HPP */
