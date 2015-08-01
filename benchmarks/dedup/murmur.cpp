/**
 * murmur.cpp - 
 * @author: Jonathan Beard
 * @version: Thu Jul 30 20:34:03 2015
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
#include "murmur.hpp"

std::uint32_t 
murmur::murmur3_32( const char * const key, 
                    const std::uint32_t len, 
                    const std::uint32_t seed) 
{
   using murm_t = std::uint32_t;
	static const murm_t c1( 0xcc9e2d51);
	static const murm_t c2( 0x1b873593);
	static const murm_t r1( 15);
	static const murm_t r2( 13);
	static const murm_t m ( 5);
	static const murm_t n ( 0xe6546b64);

	auto  hash( seed );

	const murm_t  nblocks( len >> 2 );
	const murm_t * const blocks( 
      reinterpret_cast< const murm_t * const >( key )
   );
	for( auto i( 0 ); i < nblocks; i++ ) 
   {
		murm_t k( blocks[ i ] );
		k *= c1;
		k = (k << r1) | (k >> (32 - r1));
		k *= c2;

		hash ^= k;
		hash = ((hash << r2) | (hash >> (32 - r2))) * m + n;
	}

	const std::uint8_t * const tail( 
      reinterpret_cast< const std::uint8_t * const >( key + nblocks * 4 )
   );
	murm_t k1( 0 );

	switch( len & 3 ) 
   {
      case( 3 ):
         k1 ^= tail[2] << 16;
      case( 2 ):
         k1 ^= tail[1] << 8;
      case( 1 ):
         k1 ^= tail[0];
         k1 *= c1;
         k1 = (k1 << r1) | (k1 >> (32 - r1));
         k1 *= c2;
         hash ^= k1;
	}

	hash ^= len;
	hash ^= (hash >> 16);
	hash *= 0x85ebca6b;
	hash ^= (hash >> 13);
	hash *= 0xc2b2ae35;
	hash ^= (hash >> 16);
	return( hash );
}
