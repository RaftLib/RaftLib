/**
 * port.hpp - 
 * @author: Jonathan Beard
 * @version: Thu Aug 28 09:55:47 2014
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
#ifndef _PORT_HPP_
#define _PORT_HPP_  1

#include <map>
#include <string>

#include "fifo.hpp"
#include "portexception.hpp"

class Port
{
public:
   Port();
   virtual ~Port();

   /**
    * addPort - adds and initializes a port for the name
    * given.  Function returns true if added, false if not.
    * Main reason for returning false would be that the 
    * port already exists.
    * @param   port_name - const std::string
    * @return  bool
    */
   bool  addPort( const std::string port_name );
   /**
    * getPortType - input the port name, and get the hash
    * for the type of the port.  This function is useful
    * for checking the streaming graph to make sure all the
    * ports that are "dynamically" created do in fact have
    * compatible types.
    * @param port_name - const std::string
    * @return  std::size_t - hash code for port type
    */
   std::size_t getPortType( const std::string port_name );

   /**
    * operator[] - input the port name and get a port
    * if it exists. 
    */
   FIFO& operator[]( const std::string port_name );

protected:
   std::map< std::string, FIFO* > portmap;   
};
#endif /* END _PORT_HPP_ */
