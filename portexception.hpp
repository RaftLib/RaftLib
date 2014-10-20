/**
 * portexception.hpp - 
 * @author: Jonathan Beard
 * @version: Wed Sep  3 14:52:27 2014
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
#ifndef _PORTEXCEPTION_HPP_
#define _PORTEXCEPTION_HPP_  1
#include <exception>
#include <string>
class PortException : public std::exception
{
public:
   PortException( const std::string message );
   virtual const char* what() const noexcept;
private:
   std::string message;
};

class PortTypeException : public PortException
{
public:
   PortTypeException( const std::string message );
};

class PortNotFoundException : public PortException
{
public:
   PortNotFoundException( const std::string message );
};

class PortDoubleInitializeException : public PortException
{
public:
   PortDoubleInitializeException( const std::string message );
};

class PortTypeMismatchException : public PortException
{
public:
   PortTypeMismatchException( const std::string message );
};

class AmbiguousPortAssignmentException : public PortException
{
public:
   AmbiguousPortAssignmentException( const std::string message );
};

class ClosedPortAccessException : public PortException
{
public:
   ClosedPortAccessException( const std::string message );
};
#endif /* END _PORTEXCEPTION_HPP_ */
