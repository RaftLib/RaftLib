#include "port_info.hpp"

PortInfo::PortInfo() : type( typeid( (*this) ) )
{
}

PortInfo::PortInfo( const std::type_info &the_type ) : type( the_type )
{
}

PortInfo::PortInfo( const std::type_info &the_type,
                    void * const ptr,
                    const std::size_t nitems,
                    const std::size_t start_index )  : type( the_type ),
                                                existing_buffer( ptr ),
                                                nitems( nitems ),
                                                start_index( start_index )
{

}


PortInfo::PortInfo( const PortInfo &other ) : type( other.type )
{
   fifo_a         = other.fifo_a;
   fifo_b         = other.fifo_b;
   const_map      = other.const_map;
   my_kernel      = other.my_kernel;
   my_name        = other.my_name;
   other_kernel   = other.other_kernel;
   other_name     = other.other_name;
   out_of_order   = other.out_of_order;
   existing_buffer= other.existing_buffer;
   nitems         = other.nitems;
   start_index    = other.start_index;
   split_func      = other.split_func;
   join_func       = other.join_func;
   fixed_buffer_size = other.fixed_buffer_size;
}


PortInfo::~PortInfo()
{
   /** alloc delete fifo object **/
}


FIFO* 
PortInfo::getFIFO()
{
   struct{
      FIFO *a;
      FIFO *b;
   }copy = { fifo_a, fifo_b };
   /** for most architectures that don't need this, it'll be optimized out after the first iteration **/
   while( copy.a != copy.b )
   {
      copy.a = fifo_a;
      copy.b = fifo_b;
   }
   return( copy.a );
}

void 
PortInfo::setFIFO( FIFO * const in )
{
   assert( in != nullptr );
   fifo_a = in;
   fifo_b = in;
}
