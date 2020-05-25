/**
 * read_each.tcc - 
 * @author: Jonathan Beard
 * @version: Sun Oct 26 15:51:46 2014
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
#ifndef RAFTREADEACH_TCC
#define RAFTREADEACH_TCC  1
#include <iterator>
#include <functional>
#include <map>
#include <cstddef>

#include <raft>

namespace raft{


template < class T, 
           class Iterator > class readeach : public parallel_k
{
public:
    readeach( Iterator &b,
              Iterator &e ) : parallel_k(),
                              begin( b ),
                              end( e )
    {
        addPortTo< T >( output );
    }

    
    readeach( const readeach &other ) : parallel_k(),
                                        begin( std::move( other.begin ) ),
                                        end  ( std::move( other.end ) )
    {
        addPortTo< T >( output );
    }
    
    virtual ~readeach() = default;


    virtual raft::kstatus run()
    {
        for( auto it( begin ); it != end; ++it )
        {
            auto val( output[ "0" ].template allocate_s< T >() );
            (*val) = (*begin);
            ++begin;
        }
        return( raft::stop );
    }

private:
    Iterator       begin;
    const Iterator end;
}; /** end template readeach **/

template < class T, 
           class Iterator > 
static
raft::readeach< T, Iterator >
read_each( Iterator &&begin, 
           Iterator &&end )
{
    return( readeach< T, Iterator >( begin,
                                     end  ) );
}


} /** end namespace raft **/
#endif /* END RAFTREADEACH_TCC */
