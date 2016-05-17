/**
 * signaldata.hpp - current version only supports single signals,
 * future versions will support more, for right now this is all 
 * it really needs to work. 
 * 
 * NOTES:
 * This class is still largely unused so
 * lets leave the attributes as is. We'll come back and fix once 
 * we actually start needing more advanced signaling, or if I'm 
 * convinced that we really don't need it (almost am now) then we'll
 * delete this and move on to grander things. - jcb 16May2016
 *
 * @author: Jonathan Beard
 * @version: Sat Jan 10 20:29:38 2015
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
#ifndef _SIGNALDATA_HPP_
#define _SIGNALDATA_HPP_  1

#include <thread>

#include "signalvars.hpp"

class SignalData
{
public:
   SignalData()          = default;
   virtual ~SignalData() = default;

   raft::signal get_signal(  const std::thread::id id );
   
   void         set_signal( const std::thread::id id, const raft::signal signal );

private:
   raft::signal __attribute__((__unused__)) signal_a = raft::none;
   raft::signal __attribute__((__unused__)) signal_b = raft::none;

};
#endif /* END _SIGNALDATA_HPP_ */
