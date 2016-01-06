/**
 * random.tcc - 
 * @author: Jonathan Beard
 * @version: Mon Jan  4 15:17:03 2016
 * 
 * Copyright 2016 Jonathan Beard
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
#ifndef _RANDOM_TCC_
#define _RANDOM_TCC_  1
/** use cpp rnd generator **/
#include <random>
#include <raft>
#include <cstddef>
#include <chrono>

namespace raft
{
template < class GENERATOR,
           template < class > class DIST,
           class TYPE >
class random_variate : public parallel_k
{
public:
    template <class ... Args > 
    random_variate( const std::size_t N, 
                    Args&&... params ) : parallel_k(),
                                         gen( std::chrono::system_clock::now().time_since_epoch().count() ),
                                         dist( std::forward< Args >( params )... ),
                                         N( N )
    {
        addPortTo< TYPE >( output );
    }

    virtual ~random_variate() = default;

    virtual raft::kstatus run()
    {
        for( auto &p : output )
        {
            p.push( dist( gen ) );
            if( ++count_of_sent >= N )
            {
                return( raft::stop );
            }
        }
        return( raft::proceed );
    }

private:
    GENERATOR       gen;
    DIST< TYPE >    dist;
    std::size_t     count_of_sent = 0;
    const std::size_t N;
};


}

#endif /* END _RANDOM_TCC_ */
