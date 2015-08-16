/**
 * acsearch.tcc - 
 * @author: Jonathan Beard
 * @version: Sun Nov  9 17:09:08 2014
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
#ifndef _ACSEARCH_TCC_
#define _ACSEARCH_TCC_  1
#include <raft>
#include <functional>

#include "ahocorasick.h"
#include "searchdefs.hpp"
#include <cassert>
struct parameter
{
   std::size_t position = 0;
   FIFO       *fifo     = nullptr;
};

static int 
match_handler( AC_MATCH_t *matchp, void *param )
{
   auto *p( reinterpret_cast< parameter* >(param) );
   p->fifo->push< hit_t >( matchp->position + p->position );
   return( 0 );
}

template <> class search< ahocorasick > : public kernel
{
public:
   search( const std::string searchterm ) : kernel()
   {
      assert( searchterm.length() > 0 ); 
      input.addPort< char  >( "in"  );
      output.addPort< hit_t   >( "out" );
      
      /** initialize automata **/
      atm = ac_automata_init();
      /** add only pattren **/
      AC_PATTERN_t tmp_patt;
      tmp_patt.astring = searchterm.c_str();
      tmp_patt.rep.number  = 0; 
      tmp_patt.length  = searchterm.length();
      ac_automata_add( atm, &tmp_patt );
      ac_automata_finalize( atm );
   }

   virtual ~search()
   {
      ac_automata_release( atm );
   }

   virtual raft::kstatus run() 
   {
      auto &in_port( input[ "in" ] );
      const auto text_length( in_port.size() );
      auto everything( in_port.peek_range< char >( text_length ) );
      /** in this case, we know the buffer is contiguous **/
      AC_TEXT_t tmp_text;
      tmp_text.astring =
         static_cast< char* >( &( everything[ 0 ] ) );
      tmp_text.length  = text_length;

      auto &out_port( output[ "out" ] );
      param.fifo     = &out_port;
      param.position = everything.getindex();
      ac_automata_search( atm, &tmp_text, 0, match_handler, (void*)&param );
      return( raft::stop );
   }
   
private:
   AC_AUTOMATA_t *atm = nullptr;   
   parameter      param;

};
#endif /* END _ACSEARCH_TCC_ */
