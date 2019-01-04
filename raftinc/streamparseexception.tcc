/**
 * streamparseexception.tcc - 
 * @author: Jonathan Beard
 * @version: Sat Dec 31 05:32:37 2016
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
#ifndef _STREAMPARSEEXCEPTION_TCC_
#define _STREAMPARSEEXCEPTION_TCC_  1

#include <string>
#include <utility> 

#include "raftexception.hpp"


class StreamParseException : public RaftException
{
public:
    StreamParseException(  const std::string &message ) : 
        RaftException( message ){};
    StreamParseException(  const std::string &&message ) : 
        RaftException(  std::move( message ) ){};

};


template < int N > class TemplateStreamParseException : public StreamParseException 
{
public:
    TemplateStreamParseException(  const std::string &message ) : 
        StreamParseException( message ){};
    
    TemplateStreamParseException(  const std::string &&message ) : 
        StreamParseException(  std::move( message ) ){};
};


/**
 * PortNotSpecifiedForKSet - thrown if the user attempts to specify
 * a connection to a kernel within a kernel set structure that has
 * more than one input or output port without specifying which port
 * to connect to. This exception is similar to the ambiguous port
 * exception above, except specific to kernel sets (kset).
 */
using PortNotSpecifiedForKSet           
    = TemplateStreamParseException< 0 >;


/**
 * RaftManipException - thrown if the programmer provides
 * a set of specifications that do not exist (either 
 * outside the range of 64 states provided, or are 
 * reserved bits.
 */
using RaftManipException 
    = TemplateStreamParseException< 1 >;
/** 
 * NonsenseChainRaftManipException - thrown if the programmer
 * provides a set fo stream modifers that make no sense 
 * together or if the user provides a set of contradictory
 * stream modifiers back to back without a kernel in between.
 */
using NonsenseChainRaftManipException
    = TemplateStreamParseException< 2 >;

#endif /* END _STREAMPARSEEXCEPTION_TCC_ */
