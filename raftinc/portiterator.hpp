/**
 * portiterator.hpp - 
 * @author: Jonathan Beard
 * @version: Sun Oct  5 08:49:11 2014
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
#ifndef _PORTITERATOR_HPP_
#define _PORTITERATOR_HPP_  1
#include <iterator>
#include <map>
#include <thread>
#include <mutex>
#include <cstddef>
#include "defs.hpp"
#include "portmap_t.hpp"

class FIFO;

class PortIterator : public std::iterator< std::forward_iterator_tag, FIFO >
{
public:
   PortIterator( portmap_t * const port_map );
   
   PortIterator( portmap_t * const port_map, const std::size_t index );

   PortIterator( const PortIterator &it );

   virtual ~PortIterator();

   PortIterator& operator++() ;
   
   bool operator==(const PortIterator& rhs) ; 
   bool operator!=(const PortIterator& rhs) ;
   FIFO& operator*() ;


   const raft::port_key_type& name();

private:
   static inline
   void initKeyMap( portmap_t * const port_map, 
                    std::vector< raft::port_key_type > &key_map ) ;
   
   portmap_t * const                    port_map;
   std::vector< raft::port_key_type >   key_map;
   std::size_t                          map_index = 0;
   bool                                 is_end       = false;
};

#endif /* END _PORTITERATOR_HPP_ */
