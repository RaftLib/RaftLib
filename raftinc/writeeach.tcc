/**
 * write_each.tcc -
 * @author: Jonathan Beard
 * @version: Sun Oct 26 15:51:46 2014
 *
 * Copyright 2014 Jonathan Beard
 * Copyright 2021 Jonathan Beard
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
#ifndef RAFTWRITEEACH_TCC
#define RAFTWRITEEACH_TCC  1
#include <iterator>
#include <raft>
#include <cstddef>
#include <typeinfo>
#include <functional>
#include <type_traits>
#include <cassert>

namespace raft{


template < class T, class BackInsert > class writeeach : public parallel_k
{
public:
    writeeach( BackInsert &bi ) : parallel_k(),
                                  inserter( bi )
    {
        addPortTo< T >( input );
    }

    writeeach( const writeeach &other ) : parallel_k(),
                                          inserter( other.inserter )
    {
        /** add a single port **/
        addPortTo< T >( input );
    }

    virtual ~writeeach() = default;

    virtual raft::kstatus run()
    {
        for( auto &port : input )
        {
            const auto avail_data( port.size() );
            static_assert( std::is_unsigned< decltype( avail_data ) >::value,
                            "avail_data size must be unsigned" );
            if( avail_data != 0 )
            {
                auto alldata( port.template peek_range< T >( avail_data ) );

                using index_type = std::remove_const_t<decltype(avail_data)>;
                for( index_type index( 0 ); index < avail_data; index++ )
                {
                   (*inserter) = alldata[ index ].ele;
                   /** hope the iterator defined overloaded ++ **/
                   ++inserter;
                }
                port.recycle( avail_data  );
            }
        }
        return( raft::proceed );
    }
private:
    BackInsert inserter;
};

template < class T, class BackInsert >
static
writeeach< T, BackInsert >
write_each( BackInsert &&bi )
{
    return( writeeach< T, BackInsert >( bi ) );
}

} /** end namespace raft **/
#endif /* END RAFTWRITEEACH_TCC */
