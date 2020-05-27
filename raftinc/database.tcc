/**
 * database.tcc - 
 * @author: Jonathan Beard
 * @version: Fri Jan  2 19:08:42 2015
 * 
 * Copyright 2015 Jonathan Beard
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
#ifndef RAFTDATABASE_TCC
#define RAFTDATABASE_TCC  1

#include "pointer.hpp"
#include "signal.hpp"
#include <cstddef>
#include "blocked.hpp"
#include "threadaccess.hpp"

namespace raft
{
   class kernel;
}

namespace Buffer
{

/**
 * DataBase - not quite the best name since we 
 * conjure up a relational database, but it is
 * literally the base for the Data structs.
 */
template < class T > struct DataBase 
{
    DataBase( const std::size_t max_cap ) : max_cap ( max_cap ),
                                            length_store( sizeof( T ) * max_cap ),
                                            length_signal( sizeof( T ) * max_cap ),
                                            dynamic_alloc_size( length_store +
                                                                length_signal )
                                            {}

    /** 
     * copyFrom - implement in all sub-structs to 
     * copy the buffer.  Might need to reinterpret
     * cast the other object or other type of cast
     * in order to get all the data members you wish
     * to copy.
     * @param   other - struct to be copied
     */
    virtual void copyFrom( DataBase< T > *other ) = 0;


    const std::size_t       max_cap;
    /** sizes, might need to define a local type **/
    const std::size_t       length_store;
    const std::size_t       length_signal;
    
    const std::size_t       dynamic_alloc_size;


    /**
     * read/write pointer. the thread_access is in 
     * between as it's well accessed by both just
     * as frequently as the pointers themselves, 
     * so we get decent caching behavior out of 
     * doing it this way. 
     */
    Pointer                 read_pt;
    ThreadAccess            thread_access[ 2 ];
    Pointer                 write_pt;
    
    /** 
     * FIXME - decide if 32 is the best choice or have
     * a macro that defines it per compile.
     */
    T                       *store          = nullptr;
    Signal                  *signal         = nullptr;
    bool                    external_alloc  = false;
    /** variable set by scheduler, used for shutdown **/
    bool                    is_valid        = true;
    
    /**
     * these keep reference over how many read/writes are
     * blocked. used for dynamic adaptation.
     */
    Blocked                 read_stats; 
    Blocked                 write_stats;
    
    /** need to force resize, this has the count requested **/
    std::size_t             force_resize    = 0;

     
    using value_type = T;

    
    const std::size_t       static_alloc_size = sizeof( DataBase< T > );
};

} /** end namespace Buffer **/
#endif /* END RAFTDATABASE_TCC */
