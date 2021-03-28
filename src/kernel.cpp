#include <stdio.h>
#include "portexception.hpp"
#include "kernel.hpp"
#include "common.hpp"

using namespace raft;

std::size_t kernel::kernel_count( 0 );

#ifdef BENCHMARK
    std::atomic< std::size_t > kernel::initialized_count( 0 );
#endif

/** default **/
kernel::kernel() : kernel_id( kernel::kernel_count )
{
   kernel::kernel_count++;
}

/** existing memory **/
kernel::kernel( void * const ptr,
                const std::size_t nbytes ) :
   input(  this, ptr, nbytes ),
   output( this, ptr, nbytes ),
   kernel_id( kernel::kernel_count )
{
}


std::size_t
kernel::get_id()
{
   return( kernel_id );
}

/** 
 * this will get entirely deprecated
 * at some future point, but, not yet
 * there are quite a few codes using just
 * bare strings. 
 */
#ifdef STRING_NAME
raft::kernel&
kernel::operator []( const raft::port_key_type &&portname )
{
   if( enabled_port.size() < 2 )
   {
        enabled_port.push( portname );
   }
   else
   {
        throw AmbiguousPortAssignmentException(
            "too many ports added with: " + portname
        );
   }
   return( (*this) );
}

raft::kernel&
kernel::operator []( const raft::port_key_type &portname )
{
   if( enabled_port.size() < 2 )
   {
        enabled_port.push( portname );
   }
   else
   {
        throw AmbiguousPortAssignmentException(
            "too many ports added with: " + portname
        );
   }
   return( (*this) );
}
#endif /** end STRING_NAME **/

std::size_t
kernel::addPort()
{
   return( 0 );
}

void
kernel::allConnected()
{
    /**
     * NOTE: would normally have made this a part of the 
     * port class itself, however, for the purposes of 
     * delivering relevant error messages this is much
     * easier.
     */
    for( auto it( input.begin() ); it != input.end(); ++it )
    {
        /** 
         * this will work if this is a string or not, name, returns a 
         * type based on what is in defs.hpp.
         */
        const auto &port_name( it.name() );
        const auto &port_info( input.getPortInfoFor( port_name ) );
        /**
         * NOTE: with respect to the inputs, the 
         * other kernel is the source arc, the 
         * my kernel is the local kernel.
         */
        if( port_info.other_kernel == nullptr )
        {
            std::stringstream ss;
            ss << "Port from edge (" << "null" << " -> " << 
                port_info.my_name << ") with kernel types (src: " << 
                "nullptr" << "), (dst: " <<
                common::printClassName( *port_info.my_kernel ) << "), exiting!!\n";

            throw PortUnconnectedException( ss.str() );
                
        }
    }

    for( auto it( output.begin() ); it != output.end(); ++it )
    {
        const auto &port_name( it.name() );
        const auto &port_info( output.getPortInfoFor( port_name ) );
        /**
         * NOTE: with respect to the inputs, the 
         * other kernel is the source arc, the 
         * my kernel is the local kernel.
         */
        if( port_info.other_kernel == nullptr )
        {
            std::stringstream ss;
            ss << "Port from edge (" << port_info.my_name << " -> " << 
                "null" << ") with kernel types (src: " << 
                common::printClassName( *port_info.my_kernel ) << "), (dst: " <<
                "nullptr" << "), exiting!!\n";
            throw PortUnconnectedException( ss.str() );
        }
    }
}

void
kernel::lock()
{
   /** does nothing, just need a base impl **/
   return;
}

void
kernel::unlock()
{
   /** does nothing, just need a base impl **/
   return;
}

raft::port_key_type
kernel::getEnabledPort()
{
    if( enabled_port.size() == 0 )
    {
        return( raft::null_port_value );
    }
    const auto head( enabled_port.front() );
    enabled_port.pop();
    return( head );
}

#ifdef STRING_NAMES
raft::kernel& kernel::operator []( const raft::port_key_type &&portname )
{
    if( enabled_port.size() < 2 )
    {
         enabled_port.push( portname );
    }
    else
    {
         throw AmbiguousPortAssignmentException(
         //    "too many ports added with: " + portname.str
             "too many ports added with: "
         );
    }
    return( (*this) );
}
raft::kernel& kernel::operator []( const raft::port_key_type &portname )
{
    if( enabled_port.size() < 2 )
    {
         enabled_port.push( portname );
    }
    else
    {
         throw AmbiguousPortAssignmentException(
               //"too many ports added with: " + portname.str
             "too many ports added with: "
         );
    }
    return( (*this) );
}
#endif
//std::string
//kernel::getName()
//{
//   int status( 0 );
//   const std::string name_a(
//      abi::__cxa_demangle( typeid( *(this) ).name(), 0, 0, &status ) );
//
//}
