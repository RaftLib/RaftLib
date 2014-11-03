/**
 * rkverifymatch.hpp - 
 * @author: Jonathan Beard
 * @version: Sun Nov  2 13:07:26 2014
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
#ifndef _RKVERIFYMATCH_HPP_
#define _RKVERIFYMATCH_HPP_  1
#include <cmath>
#include <cstddef>
#include <string>

#include <raft>

namespace raft{
struct match_t
{
   match_t()
   {
   }

   match_t( const char * const filebuffer, 
            const std::size_t match_pos,
            const std::size_t match_length )
   {
      const auto end( std::min( match_length, ( buff_length - 1 ) ) );
      seg[ end + 1 ] = '\0';
      for( std::size_t i( 0 ); i < end; i++ )
      {
         seg[ i ] = filebuffer[ match_pos + i ];
      }
      seg_length = end;
      hit_pos = match_pos;
   }

   match_t( const match_t &other )
   {
      std::size_t index( 0 );
      for( ; index < other.seg_length; index++ )
      {
         seg[ index ] = other.seg[ index ];
      }
      seg_length = other.seg_length;
      hit_pos    = other.hit_pos;
   }

   static const std::size_t buff_length = 32;
   char seg[ match_t::buff_length ];
   std::size_t seg_length = 0;
   std::size_t hit_lineno = 0;
   std::size_t hit_pos    = 0;
};

class rkverifymatch : public kernel
{
public:
   rkverifymatch( const std::string filename, const std::string term );
   virtual ~rkverifymatch();

   virtual kstatus run();

private:
   /** global single search term **/
   const std::string searchterm;
   /** filestuff for mmap **/
   char * filebuffer            = nullptr;
   std::size_t file_buffer_size = 0;
   /** functions **/

   static bool verify_match( const char * const buffer,
                             const std::size_t buff_size,
                             const std::string &term, 
                             const std::size_t position,
                             match_t   &m );
};
} /** end namespace raft **/
#endif /* END _RKVERIFYMATCH_HPP_ */
