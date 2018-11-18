/**
 * raftexception.hpp - 
 * @author: Jonathan Beard
 * @version: Fri Dec 23 13:59:44 2016
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
#ifndef _RAFTEXCEPTION_HPP_
#define _RAFTEXCEPTION_HPP_  1
#include <exception>
#include <string>

class RaftException : public std::exception
{
public:
   
    RaftException( const std::string message );
   
    virtual const char* what() const noexcept;
private:
    const char *message = nullptr;
};

/** 
 * TO MAKE AN EXCEPTION 
 * 1) MAKE AN EMPTY DERIVED CLASSS OF RaftException
 * 2) MAKE ONE OF THESE TEMPLATES WHICH DERIVES FROM THAT
 *    EMPTY CLASS
 * THEN ADD A using MYNEWEXCEPTION = Template< # >
 */
#if 0
template < int N > class TemplateRaftException : public RaftException 
{
public:
    TemplateRaftException(  const std::string &message ) : RaftException( message ){};
};
#endif 

#endif /* END _RAFTEXCEPTION_HPP_ */
