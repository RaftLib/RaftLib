/**
 * kernelexception.hpp - 
 * @author: Jonathan Beard
 * @version: Wed Sep  3 14:52:27 2014
 * 
 * Copyright 2016 Jonathan Beard
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
#ifndef _KERNELEXCEPTION_HPP_
#define _KERNELEXCEPTION_HPP_  1
#include <string>

#include "raftexception.hpp"


class KernelException : public RaftException
{
public:
    KernelException(  const std::string &message ) : 
        RaftException( message ){};
    
    KernelException(  const std::string &&message ) : 
        RaftException( std::move( message ) ){};
};


template < int N > class KernelExceptionBase : public KernelException 
{
public:
    KernelExceptionBase(  const std::string &message ) : 
        KernelException( message ){};
    
    KernelExceptionBase(  const std::string &&message ) : 
        KernelException( std::move( message ) ){};
};


using CloneNotImplementedException = KernelExceptionBase< 0 >;


#endif /* END _KERNELEXCEPTION_HPP_ */
