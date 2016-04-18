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
#include <exception>
#include <string>
class KernelException : public std::exception
{
public:
   KernelException( const std::string message ) : message( message ){};

   virtual const char* what() const noexcept;
private:
   const std::string message;
};

class CloneNotImplementedException : public KernelException
{
public:
    CloneNotImplementedException( const std::string message ) : 
        KernelException( message ){};
};


#endif /* END _KERNELEXCEPTION_HPP_ */
