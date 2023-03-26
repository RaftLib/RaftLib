/**
 * hh.hpp - Highway Hash code that is constexpr compatible, this code
 * is inspired by and derives significant amount of code from
 * https://github.com/google/highwayhash to produce a compatible hash,
 * that being said, it is an almost entirely different codebase at this
 * point being far simpler (and less advanced than the original product),
 * but it does produce a decent compile time hash function.
 *
 * @author: Jonathan Beard, Qinzhe Wu
 * @version: Wed Mar  22 09:31:48 2023
 *
 * Copyright 2023 The Regents of the University of Texas
 * Copyright 2020 Jonathan Beard
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


#ifndef CONSTEXPR_HIGHWAYHASH
#define CONSTEXPR_HIGHWAYHASH 1

/**
 * WARNING, requires C++17 to compile because the std::array operators were
 * forgotten to be marked as constexpr until C++17.
 */

#include <cstdint>
#include <cstring>
#include <array>
#include <utility>
#include <string>

class highway_hash
{
public:

struct hash_t
{
   using val_type = std::uint64_t;
};

/**
 * primary initiator for using the _64 user-defined
 * string literal operator, the constexpr constructor
 * does the heavy lifting.
 */
template< std::size_t N >
struct data_t : hash_t
{
    /**
     * NOTE: length does not include null terminator,
     * actual array size is (len + 1), this also has
     * to be static for the constexpr magic to work,
     * doesn't really matter if we have it shared given
     * it's just the length, and if it ends up being
     * shared then it's fine b/c it's the same length.
     */
    static const  decltype( N ) len         = N - 1;
    /**
     * val gets the hash value
     */
    hash_t::val_type               val         = 0;
    /**
     * on template instantiation, str[ N ] is the length
     * of the string including the null terminator.
     */
    char                        str[ N ]    = {'\0'};

    constexpr data_t( char const(&input)[N] )
    {
        for( std::size_t i( 0 ); i < N; i++ )
        {
            str[ i ] = input[ i ];
        }
        highway_hash::state state;
        highway_hash::process_all(  (*this),
                                     state );
        val = highway_hash::finalize64( state );
    }

};

/**
 * helper struct, if you need a version of the above
 * to track the actual string names in a fixed size
 * container, use this one after the above, simply
 * construct one of these, in say, a map, then you can
 * lookup strings (assuming you can bound the length
 * that is).
 */
template< std::size_t N > struct data_fixed_t : highway_hash::hash_t
{
    /**
     * does not include null term
     */
    std::size_t                     len         = 0;
    /**
     * val gets the hash value
     */
    hash_t::val_type                val         = 0;
    /**
     * on template instantiation, str[ N ] is the length
     * of the string including the null terminator.
     */
    char                        str[ N ]    = {'\0'};

    template < class T > constexpr data_fixed_t( const T &t )
    {
        std::size_t i = 0;
        for( ; i < N && i < T::len; i++ )
        {
            str[ i ] = t.str[ i ];
        }
        val = t.val;
        len = t.len;
    }

    constexpr data_fixed_t( const hash_t::val_type hash_val, const std::string input_str )
    {
        std::size_t i   = 0;
        const auto _len = input_str.length();

        for( ; i < N && i < _len; i++ )
        {
            str[i] = input_str[i];
        }
        val = hash_val;
    }
};

/**
 * if you need the fixed length version, I'd recommend you
 * simply make a new name with "using" and then set the
 * array length with the template parameter. Like so:
 * ```cpp
 * using xxname_t = data_fixed_t< 64 >;
 * ```
 */


/**
 * simplified version for data_t, avoiding making buffer
 * length N as the templare parameter.
 */
struct easy_t : hash_t
{
    hash_t::val_type               val         = 0;

    constexpr easy_t( const char *input, std::size_t len )
    {
        highway_hash::state state;
        highway_hash::process_all(  input,
                                    len,
                                    state );
        val = highway_hash::finalize64( state );
    }
};

private:
/**
 * code and keys adapted from library code
 * https://github.com/google/highwayhash,
 * although significantly re-written to enable
 * c++20 features and constexpr hashing of
 * strings.
 */
constexpr static std::uint64_t _the_key[4] = {
  0x0706050403020100ull,
  0x0F0E0D0C0B0A0908ull,
  0x1716151413121110ull,
  0x1F1E1D1C1B1A1918ull
};


/**
 * read_64b_t is basically a 64b vector of 8-byte
 * chars.
 */
using read_64b_t = std::array< std::uint8_t, 8>;
/**
 * read_32B_t - a 32B vector of 32 chars.
 */
using read_32B_t = std::array< std::uint8_t, 32>;

/**
 * convert - converts the 32B char input to a 64b output
 * starting at the param offset. Note: this function
 * could fail if the offset > 24, so sanitize input before
 * arriving here.
 * @param input - read_32B_t (32 char input)
 * @param offset - int, offset w.r.t. 32B input to start at.
 */
constexpr static
read_64b_t convert ( const read_32B_t &input, const int offset ) noexcept
{
    return( read_64b_t( {    input[ 0 + offset ],
                             input[ 1 + offset ],
                             input[ 2 + offset ],
                             input[ 3 + offset ],
                             input[ 4 + offset ],
                             input[ 5 + offset ],
                             input[ 6 + offset ],
                             input[ 7 + offset ] } ) );
}

/**
 * convert32B - takes an arbitrary size input string (static char
 * array) and outputs a 32B (32 char) chunk of that input starting
 * at the param offset and running to len in bytes.
 * @param t - data_t class structure from user-defined str literal,
 * @param offset - offset w.r.t. 't' for index 0 of output 32B,
 * @param len - number of data elements from 't' to place into output.
 * @return  read_32B_t.
 */
template < class T >
constexpr static
read_32B_t  convert32B( const T &t,
                        const std::size_t offset,
                        const std::size_t len ) noexcept
{
    read_32B_t temp = { '\0' };
    std::size_t i = offset;
    std::size_t j = 0;
    for( ; i < T::len && j < len; i++, j++ )
    {
        temp[ j ] = t.str[ i ];
    }
    return( temp );
}

struct state
{
    using type_t = std::array< std::uint64_t, 4 >;
    constexpr state() = default;
    type_t v0       = { 0 };
    type_t v1       = { 0 };
    type_t mul0     = { 0 };
    type_t mul1     = { 0 };
};

/* Initializes state with given key */
static
constexpr
void reset( highway_hash::state &state )
{
    const auto key( highway_hash::_the_key );
    state.mul0[0] = 0xdbe6d5d5fe4cce2full;
    state.mul0[1] = 0xa4093822299f31d0ull;
    state.mul0[2] = 0x13198a2e03707344ull;
    state.mul0[3] = 0x243f6a8885a308d3ull;
    state.mul1[0] = 0x3bd39e10cb0ef593ull;
    state.mul1[1] = 0xc0acf169b5f18a8cull;
    state.mul1[2] = 0xbe5466cf34e90c6cull;
    state.mul1[3] = 0x452821e638d01377ull;
    state.v0[0] = state.mul0[0] ^ key[0];
    state.v0[1] = state.mul0[1] ^ key[1];
    state.v0[2] = state.mul0[2] ^ key[2];
    state.v0[3] = state.mul0[3] ^ key[3];
    state.v1[0] = state.mul1[0] ^ ((key[0] >> 32) | (key[0] << 32));
    state.v1[1] = state.mul1[1] ^ ((key[1] >> 32) | (key[1] << 32));
    state.v1[2] = state.mul1[2] ^ ((key[2] >> 32) | (key[2] << 32));
    state.v1[3] = state.mul1[3] ^ ((key[3] >> 32) | (key[3] << 32));
}


/* Takes a packet of 32 bytes */
constexpr
static
void update_packet( const highway_hash::read_32B_t &packet,
                    highway_hash::state &state )
{
    highway_hash::state::type_t lanes = {0};
    lanes[0] = highway_hash::read64( convert( packet, 0 ) );
    lanes[1] = highway_hash::read64( convert( packet, 8 ) );
    lanes[2] = highway_hash::read64( convert( packet, 16) );
    lanes[3] = highway_hash::read64( convert( packet, 24) );
    highway_hash::update(lanes, state);
}


/* Adds the final 1..31 bytes, do not use if 0 remain */
constexpr
static
void update_remainder(    const highway_hash::read_32B_t &bytes,
                          const std::size_t        size_mod32,
                          highway_hash::state &state )
{
    const std::uint64_t size_mod4( size_mod32 & 3 );
    const std::uint64_t remainder = size_mod32 & ~3;
    highway_hash::read_32B_t packet = { '\0' };
    for( auto i( 0 ); i < 4; ++i )
    {
        state.v0[ i ] += ((std::uint64_t)size_mod32 << 32) + size_mod32;
    }
    highway_hash::rotate_32_by( size_mod32, state.v1 );
    for( std::uint64_t i( 0 );
        i < remainder /** now zero indexed, don't sub bytes **/; i++ )
    {
        packet[ i ] = bytes[ i ];
    }
    if(size_mod32 & 16)
    {
        for ( auto i( 0 ); i < 4; i++) {
            packet[28 + i] = bytes[ remainder + i + size_mod4 - 4 ];
        }
    }
    else
    {
        if( size_mod4 )
        {
            packet[16 + 0] = bytes[ remainder + 0  ];
            packet[16 + 1] = bytes[ remainder +  (size_mod4 >> 1) ];
            packet[16 + 2] = bytes[ remainder +  (size_mod4 -  1)  ];
        }
    }
    highway_hash::update_packet(packet, state);
    return;
}

/* Compute final hash value. Makes state invalid. */
constexpr
static
std::uint64_t finalize64( highway_hash::state &state )
{
    for( auto i( 0 ); i < 4; i++ )
    {
        highway_hash::permute_and_update( state );
    }
    return( state.v0[0] + state.v1[0] + state.mul0[0] + state.mul1[0] );
}



template < class T >
constexpr
static
void
process_all( T &t,
             highway_hash::state &state)
{
    std::uint64_t i( 0 );
    const auto size( t.len );
    highway_hash::reset( state );
    for(; i + 32 <= size; i += 32)
    {
        highway_hash::update_packet( convert32B( t, i, 32 ), state );
    }
    if( (size & 31) != 0 )
    {
        constexpr auto len( size & 31 );
        highway_hash::update_remainder( convert32B( t, i, len ), len, state);
    }
}


constexpr
static
void
process_all( const char *input,
             std::size_t len,
             highway_hash::state &state)
{
    std::size_t i( 0 );
    highway_hash::reset( state );

    read_32B_t temp = { '\0' };

    for(; i + 32 <= len;)
    {
        std::size_t j = 0;
        for(; j < 32; i++, j++)
        {
            temp[ j ] = input[ i ];
        }
        highway_hash::update_packet( temp, state );
    }
    if( (len & 31) != 0 )
    {
        std::size_t j = 0;
        for(; i < len; i++, j++)
        {
            temp[ j ] = input[ i ];
        }
        for(; j < 32; j++)
        {
            temp[ j ] = '\0';
        }
        highway_hash::update_remainder( temp, len & 31, state);
    }
}



constexpr
static
void
permute_and_update( highway_hash::state &state )
{
    highway_hash::state::type_t permuted = {0};
    highway_hash::permute(  state.v0, permuted );
    highway_hash::update( permuted, state );
    return;
}


static
constexpr
void
permute( const highway_hash::state::type_t &v, highway_hash::state::type_t &permuted)
{
  permuted[0] = (v[2] >> 32) | (v[2] << 32);
  permuted[1] = (v[3] >> 32) | (v[3] << 32);
  permuted[2] = (v[0] >> 32) | (v[0] << 32);
  permuted[3] = (v[1] >> 32) | (v[1] << 32);
}

static
constexpr
void
rotate_32_by( const std::uint64_t count, highway_hash::state::type_t &lanes )
{
  for( auto i( 0 ); i < 4; i++ )
  {
    const std::uint32_t half0( lanes[i] & 0xffffffff );
    const std::uint32_t half1( lanes[i] >> 32 );
    lanes[i] = (half0 << count) | (half0 >> (32 - count));
    lanes[i] |= (std::uint64_t)((half1 << count) | (half1 >> (32 - count))) << 32;
  }
}

static
constexpr
std::uint64_t
read64( const highway_hash::read_64b_t &src)
{
  return (std::uint64_t)src[0] | ((std::uint64_t)src[1] << 8) |
      ((std::uint64_t)src[2] << 16) | ((std::uint64_t)src[3] << 24) |
      ((std::uint64_t)src[4] << 32) | ((std::uint64_t)src[5] << 40) |
      ((std::uint64_t)src[6] << 48) | ((std::uint64_t)src[7] << 56);
}

static
constexpr
void
update( const highway_hash::state::type_t lanes,
        highway_hash::state               &state)
{
  for( auto i( 0 ); i < 4; ++i)
  {
    state.v1[i] += state.mul0[i] + lanes[i];
    state.mul0[i] ^= (state.v1[i] & 0xffffffff) * (state.v0[i] >> 32);
    state.v0[i] += state.mul1[i];
    state.mul1[i] ^= (state.v0[i] & 0xffffffff) * (state.v1[i] >> 32);
  }
  highway_hash::zipper_merge_and_add(state.v1[1], state.v1[0], state.v0[1], state.v0[0]);
  highway_hash::zipper_merge_and_add(state.v1[3], state.v1[2], state.v0[3], state.v0[2]);
  highway_hash::zipper_merge_and_add(state.v0[1], state.v0[0], state.v1[1], state.v1[0]);
  highway_hash::zipper_merge_and_add(state.v0[3], state.v0[2], state.v1[3], state.v1[2]);
}


static
constexpr
void
zipper_merge_and_add(  const std::uint64_t v1,
                       const std::uint64_t v0,
                       std::uint64_t      &add1,
                       std::uint64_t      &add0 )
{
  add0 += (((v0 & 0xff000000ull) | (v1 & 0xff00000000ull)) >> 24) |
           (((v0 & 0xff0000000000ull) | (v1 & 0xff000000000000ull)) >> 16) |
           (v0 & 0xff0000ull) | ((v0 & 0xff00ull) << 32) |
           ((v1 & 0xff00000000000000ull) >> 8) | (v0 << 56);
  add1 += (((v1 & 0xff000000ull) | (v0 & 0xff00000000ull)) >> 24) |
           (v1 & 0xff0000ull) | ((v1 & 0xff0000000000ull) >> 16) |
           ((v1 & 0xff00ull) << 24) | ((v0 & 0xff000000000000ull) >> 8) |
           ((v1 & 0xffull) << 48) | (v0 & 0xff00000000000000ull);
}

};


#if __cpp_nontype_template_args < 201911

static
constexpr
struct highway_hash::easy_t
operator""_64( const char *buf, std::size_t len )
{
    return highway_hash::easy_t( buf, len );
}

#else

/**
 * use this to get a constexpr 64b unsigned hash
 * of a string. Must compile with C++20 for this to
 * work given it requires return type template type
 * deduction for user-defined string literals.
 * e.g. "foobar"_64, hashes the string at compile
 * time. Currently only g++ has this capability, maybe
 * the latest head of clang, apple clang does not.
 * The return type is a struct with the string,
 * the length, and the hash value. e.g.
 * auto data( "foobar"_64 ); then the field data.val
 * contains your hash.
 */
template < highway_hash::data_t data >
static
constexpr
auto
operator""_64()
{
    return( data );
}

#endif // end of #if __cpp_nontype_template_args < 201911

#endif // end of #ifndef CONSTEXPR_HIGHWAYHASH
