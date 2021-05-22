/**
 * print.tcc - 
 * @author: Jonathan Beard
 * @version: Tue Sep 30 10:46:39 2014
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
#ifndef RAFTPRINT_TCC
#define RAFTPRINT_TCC  1

#include <functional>
#include <ostream>
#include <iostream>
#include <raft>
#include <cstdlib>

namespace raft{

class printbase
{
protected:
   std::ostream *ofs = nullptr;
};


template< typename T > class printabstract : public raft::kernel, 
                                             public raft::printbase
{
public:
    printabstract( const std::size_t n_input_ports = 1 ) : raft::kernel(),
                                                           raft::printbase()
    {
        using index_type = std::remove_const_t< decltype( n_input_ports ) >;
        for( index_type index( 0 ); index < n_input_ports; index++ )
        {
           /** add a port for each index var, all named "input_#" **/
#ifdef STRING_NAMES           
           input.addPort< T  >( std::to_string( index ) );
#else
           /**
            * if not strings, the addPort function expects a port_key_name_t struct,
            * so, we have to go and add it. 
            */
           input.addPort< T >( raft::port_key_name_t( index, std::to_string( index ) ) );
#endif
        }
        ofs = &(std::cout);
    }
    
    printabstract( std::ostream &stream, 
                   const std::size_t n_input_ports = 1 )  : raft::kernel(),
                                                            raft::printbase()
    {
        using index_type = std::remove_const_t< decltype( n_input_ports ) >;
        for( index_type index( 0 ); index < n_input_ports; index++ )
        {
           /** add a port for each index var, all named "input_#" **/
#ifdef STRING_NAMES           
           input.addPort< T  >( std::to_string( index ) );
#else
           /**
            * if not strings, the addPort function expects a port_key_name_t struct,
            * so, we have to go and add it. 
            */
           input.addPort< T >( raft::port_key_name_t( index, std::to_string( index ) ) );
#endif
        }
        ofs = &stream;
    }

protected:
    const std::size_t input_port_count    = 1;
};

template< typename T, char delim = '\0' > class print : public printabstract< T >
{
public:
    print( const std::size_t n_input_ports = 1 ) : printabstract< T >( n_input_ports )
    {
    }
    
    print( std::ostream &stream, 
           const std::size_t n_input_ports = 1 ) : printabstract< T >( stream, 
                                                                       n_input_ports )
    {
    }
    
    print( const print &other ) : print( *other.ofs, other.input_port_count )
    {
    }


    /** enable cloning **/
    CLONE();

    /** 
     * run - implemented to take a single 
     * input port, pop the itam and print it.
     * the output isn't yet synchronized so if
     * multiple things are printing to std::cout
     * then there might be issues, otherwise
     * this works well for debugging and basic 
     * output.
     * @return raft::kstatus
     */
    virtual raft::kstatus run()
    {
        for( auto &port : (this)->input )
        {
            if( port.size() > 0 )
            {
                const auto &data( port.template peek< T >() );
                *((this)->ofs) << data << delim;
                port.unpeek();
                port.recycle( 1 );
            }
        }
        return( raft::proceed );
    }
};


template< typename T > class print< T, '\0' > : public printabstract< T >
{
public:
    print( const std::size_t n_input_ports = 1 ) : printabstract< T >( n_input_ports )
    {
    }
    
    print( std::ostream &stream, 
           const std::size_t n_input_ports = 1 ) : printabstract< T >( stream, 
                                                                       n_input_ports )
    {
    }
    
    print( const print &other ) : print( *other.ofs, other.input_port_count )
    {
    }

    CLONE();
                                 

    /** 
     * run - implemented to take a single 
     * input port, pop the itam and print it.
     * the output isn't yet synchronized so if
     * multiple things are printing to std::cout
     * then there might be issues, otherwise
     * this works well for debugging and basic 
     * output.
     * @return raft::kstatus
     */
    virtual raft::kstatus run()
    {
        for( auto &port : (this)->input )
        {
            if( port.size() > 0 )
            {
                const auto &data( port.template peek< T >() );
                *((this)->ofs) << data;
                port.unpeek();
                port.recycle( 1 );
            }
        }
        return( raft::proceed );
    }
};

} /* end namespace raft */
#endif /* END RAFTPRINT_TCC */
