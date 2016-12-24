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

#include "raftexception.hpp"

using PortException = TemplateRaftException< __COUNTER__ >;

/**
 * PortNotFoundException - throw me if there isn't a port by the
 * given name on the input nor output port.
 */
using PortNotFoundException             
    = TemplateRaftException< __COUNTER__  >;
/**
 * PortDoubleInitializeException - throw me if the port has been 
 * doubly allocated somehow..this should likely be caught internal
 * to the runtime but worst case it gets thrown out to the user
 * to likely file a bug report with.
 */
using PortDoubleInitializeException     
    = TemplateRaftException< __COUNTER__ >; 
/**
 * PortTypeMismatchException - use me if there is a port type mis-match
 * ports in raftlib are checked dynamically...given we could have
 * dynamic type conversion, we should allow this, however throw
 * this exception if the types are totally not compatible. 
 */
using PortTypeMismatchException         
    = TemplateRaftException< __COUNTER__ >;
/**
 * AmbiguousPortAssignmentException - throw me if there is a link
 * without a specified name to a source or destination kernel that
 * has multiple potential mates for the link, hence ambiguous. These
 * links must have names. 
 */
using AmbiguousPortAssignmentException  
    = TemplateRaftException< __COUNTER__ >; 
/**
 * ClosedPortAccessException - Throw me if the user attempts an access
 * to a port that has been closed, i.e. if the upstream kernels are 
 * not going to send anymore data then the port is closed. This could
 * happen for many different reasons though so there is another exception
 * to catch just the case of no more data. 
 */
using ClosedPortAccessException         
    = TemplateRaftException< __COUNTER__ >;
/**
 * NoMoreDataException - case of no more data to be sent...if a user
 * tries to access the port when nothing more is coming then we throw
 * this exception.
 */
using NoMoreDataException               
    = TemplateRaftException< __COUNTER__ >; 
/**
 * PortAlreadyExists - the port must exist only once, i.e. not conflict
 * for the input or output to a kernel. Therefore each prot name must
 * be unique for a given side, and if not this exception is thrown. 
 */
using PortAlreadyExists                 
    = TemplateRaftException< __COUNTER__ >;
/**
 * PortNotSpecifiedForKSet - thrown if the user attempts to specify
 * a connection to a kernel within a kernel set structure that has
 * more than one input or output port without specifying which port
 * to connect to. This exception is similar to the ambiguous port
 * exception above, except specific to kernel sets (kset).
 */
using PortNotSpecifiedForKSet           
    = TemplateRaftException< __COUNTER__ >;


#endif
