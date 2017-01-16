#include "portmap_t.hpp"


portmap_t::portmap_t( const portmap_t &other ) : map( other.map )
{

}


portmap_t&
portmap_t::operator = ( const portmap_t &other )
{
    (this)->map = other.map;
    return( *this );
}
