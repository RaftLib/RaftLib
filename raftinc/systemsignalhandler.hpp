/**
 * systemsignalhandler.hpp - container for signal handlers,
 * also handles calling of the signal handler, assumes that
 * the handler has all the state it needs to execute.
 *
 * @author: Jonathan Beard
 * @version: Sat Dec  6 18:19:13 2014
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
#ifndef RAFTSYSTEMSIGNALHANDLER_HPP
#define RAFTSYSTEMSIGNALHANDLER_HPP  1
#include <exception>
#include <map>
#include <string>
#include "signalvars.hpp"
#include "systemsignal.hpp"

/**
 * simple exception for when an exception handler is 
 * expected to be defined but actually is not.
 */
class NoSignalHandlerFoundException : public std::exception
{
public:
   NoSignalHandlerFoundException( const std::string message );
   virtual const char* what() const noexcept;

private:
   std::string message;
};


class SystemSignalHandler
{
public:
   SystemSignalHandler()          = default;
   virtual ~SystemSignalHandler() = default;

   /**
    * addHandler - adds the signal handler 'handler'
    * to this container.  If the handler already 
    * exists for that signal then the last one to 
    * get added supercedes it (i.e., we're relying
    * on the base class to be called first and derived
    * classes to potentially add newer handlers).
    * @param   signal - const raft::signal
    * @param   handler - sighandler
    */
   void addHandler( const raft::signal signal, 
                    sighandler handler );

   /**
    * callHandler - calls the handler for the param signal,
    * an exception is thrown if the signal doesn't have a 
    * handler and a sigterm is passed throughout the system.
    * @param   signal - const raft::signal
    * @param   fifo   - FIFO& current port that called the signal
    * @param   kernel - raft::kernel*, currently called kernel
    * @param   data   - void*
    * @returns raft::kstatus - returns whatever the handler says otherwise proceed
    * @throws  NoSignalHandlerFoundException
    */
   raft::kstatus callHandler( const raft::signal signal,
                              FIFO               &fifo,
                              raft::kernel       *kernel,
                              void *data );

private:
   std::map< raft::signal, 
             sighandler >  handlers;
};
#endif /* END RAFTSYSTEMSIGNALHANDLER_HPP */
