/** for page size defs  **/
#ifdef __unix__	
#include <unistd.h>	
#endif
/** for numeric limits  **/
#include <limits>
/** for std::size_t     **/
#include <cstddef>

/** place all of this in raft **/
namespace raft
{
/** and inside namespace test **/
namespace test
{

struct base_non_trivial_object
{
    using ptr_t = std::int64_t;
    using self_t = const base_non_trivial_object;

    base_non_trivial_object()
    {
    }
    
    base_non_trivial_object( ptr_t *value )
    {
        if( value != nullptr )
        {
            /** 
             * silly bug...fixed, was assigning  
             * value <- value vs. setting this 
             * likely introduced when we made this
             * test case generic, fixed now. was causing
             * nonTrivialAllocatePop to fail 
             * -jcb 1 Sept 2019
             */
            (this)->value = value;
        }
    }

    ~base_non_trivial_object()
    {
        /** no cast needed, should return a 'ptr' val **/
        *(this)->value = std::numeric_limits< ptr_t >::max();
    }

    constexpr base_non_trivial_object( self_t &other )
    {
        /** let's pass the pointer to this object so we can see if it's deleted **/
        (this)->value = other.value;
    }

    constexpr base_non_trivial_object& operator= ( self_t &other )
    {
        (this)->value = other.value;
        return( *this );
    }

    ptr_t *value = nullptr;
};


/**
 * for test cases, default gets  you a big pad to initiate external
 * allocation, otherwise specify the size directly. 
 */
template < bool PAD > struct non_trivial_object : base_non_trivial_object{
};

template <> struct non_trivial_object<true> : base_non_trivial_object
{
    non_trivial_object() : base_non_trivial_object(){};
    
    non_trivial_object( base_non_trivial_object::ptr_t *ptr ) : base_non_trivial_object( ptr ){};
    
    char padding[ 
#ifdef PAGE_SIZE
    PAGE_SIZE << 1 
#elif defined (_SC_PAGESIZE )
    _SC_PAGESIZE << 1
#else
    1 << 30
#endif
    ];
};

template <> struct non_trivial_object<false> : base_non_trivial_object
{   
    non_trivial_object() : base_non_trivial_object(){};
    
    non_trivial_object( base_non_trivial_object::ptr_t *ptr ) : base_non_trivial_object( ptr ){};
    /** nothing, left just in case we wanted to modify later **/
};

} /** end namespace test **/

} /** end namespace raft **/
