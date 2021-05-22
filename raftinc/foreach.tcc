/**
 * foreach.tcc - 
 * @author: Jonathan Beard
 * @version: Fri Nov  7 09:54:51 2014
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
#ifndef RAFTFOREACH_TCC
#define RAFTFOREACH_TCC  1
#include <raft>
namespace raft
{
template < class T > class for_each : public kernel
{
public:
   for_each( T * const ptr, 
             const std::size_t nitems,
             const std::size_t nports = 0 ) : kernel( ptr, 
                                                      nitems * sizeof( T ) )
   {
      /** if nports == 0, then runtime will specify **/
      output.addInPlacePorts< T >( nports ); 
      /** no input ports since we're using the existing allocation **/
   }
   
   virtual raft::kstatus run() override
   { 
      return( raft::stop ); 
   }

};
} /** end namespace raft **/
#endif /* END RAFTFOREACH_TCC */
