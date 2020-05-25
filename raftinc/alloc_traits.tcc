#include <iostream>
#include <cstdlib>
#include <cstdint>
#include <type_traits>
#include <typeinfo>

#ifndef ALLOC_TRAITS
#define ALLOC_TRAITS 1


/**
 * TODO, make a class to abstract figuring out the 
 * move/copy/allocate semantics...it's getting far
 * go crowed in both the fifo.hpp and ringbufferheap.tcc
 * files, and therefore confusing.
 */

//FIXME, add std::move for int_allocate obj
#ifndef L1D_CACHE_LINE_SIZE

#ifdef _MSC_VER
#define STRINGIZE_HELPER(x) #x
#define STRINGIZE(x) STRINGIZE_HELPER(x)
#define WARNING(desc) message(__FILE__ "(" STRINGIZE(__LINE__) ") : Warning: " #desc)
#pragma WARNING(Using 64 bytes as default cache line size, to fix recompile with -DL1D_CACHE_LINE_SIZE=XXX)
#else
#warning "Using 64 bytes as default cache line size, to fix recompile with -DL1D_CACHE_LINE_SIZE=XXX"
#endif

#define L1D_CACHE_LINE_SIZE 64

#endif
/**
 * see if the given class/type fits within a single cache line
 */
template < class T > 
struct fits_in_cache_line : 
   std::integral_constant< bool, ( sizeof( T ) <= L1D_CACHE_LINE_SIZE ) >{};

/**
 * NOW FOR ALLOC CHECKS
 */

/**
 * inline_class_alloc, true if less than cache and has a ctor
 */
template < class T >
struct inline_class_alloc : std::integral_constant< bool, 
         fits_in_cache_line< T >::value &&
         std::is_class< T >::value >{};


/** cover the other two cases for non-ctor mem alloc **/
template < class T >
struct inline_nonclass_alloc : std::integral_constant< bool,
      fits_in_cache_line< T >::value &&
      ( std::is_array< T >::value || std::is_fundamental< T >::value ) >{};


/** cover both inline cases **/
template < class T >
struct inline_alloc : std::integral_constant< bool,
    inline_class_alloc< T >::value || inline_nonclass_alloc< T >::value >{};


/** covers classes & constructables that are bigger than cache **/
template < class T >
struct ext_class_alloc : std::integral_constant< bool, 
         ! fits_in_cache_line< T >::value &&
         std::is_class< T >::value >{};

/** designed for array types **/
template < class T >
struct ext_mem_alloc : std::integral_constant< bool,
         ! fits_in_cache_line< T >::value &&
         std::is_array< T >::value >{};

template < class T >
struct ext_alloc : std::integral_constant< bool,
         ext_class_alloc< T >::value ||
         ext_mem_alloc< T >::value >{};



/**
 * NOW FOR MOVE/PUSH CHECKS
 */
#endif
