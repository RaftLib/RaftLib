#ifndef RAFTPREFETCH_HPP
#define RAFTPREFETCH_HPP
#include <cstddef>
#include <cstdint>
#include <cassert>
#include <type_traits>
#include "defs.hpp"
namespace raft
{
/**
 * enums for prefetch, read/write pretty self explanatory
 */
enum MemAction : std::int32_t { READ= 0, WRITE = 1 };
/**
 * locality hints also pretty self explanatory 
 */
enum Locality  : std::int32_t { NO = 0, LOW = 1, MOD = 2, LOTS = 3 };

/**
 * prefetch - wrapper around built-in macro for prefetch capable of
 * fetching ranges of data. the basic idea is that this will inline
 * the prefetch insn and be a bit more intuitive than the builtin
 * functions going forward. The wrapper also enables insertion of 
 * custom ASM if ever needed without modifying more of the code
 * base.
 * @param   addr - address to prefetch
 * @return  void
 */
template< MemAction W,
          Locality  T,
          std::size_t NBYTES >
static void
prefetch( const void * const addr ) noexcept 
{
#if (defined __linux )
#ifndef NOPREFETCH    
    /** assert all the time, well except when turned off **/
    assert( addr != nullptr );
    static_assert( NBYTES > 0, "Number of bytes must be greater than zero" );
#ifndef DEBUG
    constexpr const std::uint16_t stride( L1D_CACHE_LINE_SIZE );
  
    auto start( reinterpret_cast< const char * >( addr ) );
    static_assert( std::is_unsigned< decltype( NBYTES ) >::value,
                   "NBYTES must be an unsigned type, else conditional on loop might not hold" );
    auto end( reinterpret_cast< const char * const >(
        reinterpret_cast< std::uintptr_t >( addr ) +
        NBYTES ) );
    assert( end   != nullptr );
    assert( start != nullptr );
    for( auto cp( reinterpret_cast< const char * >( start) ) ; cp < end; cp += stride )
    {
        __builtin_prefetch( cp          /** address to prefetch **/, 
                            W           /** read / write **/, 
                            T           /** temporal locality, see above **/ );
    }
#endif //end DEBUG
#endif //NOPREFETCH
#else //end APPLE AND LINUX, everyone else, empty function
    UNUSED( addr );
#endif
    return;
} /** end func **/
} /** end namespace raft **/
#endif //end prefetch.hpp
