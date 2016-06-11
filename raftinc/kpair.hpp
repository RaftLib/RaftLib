/**
 * kpair.hpp - 
 * @author: Jonathan Beard
 * @version: Wed Dec  9 11:36:08 2015
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
#ifndef _KPAIR_HPP_
#define _KPAIR_HPP_  1

#include <string>
#include "kset.tcc"
#include "portorder.hpp"
#include "defs.hpp"



namespace raft
{
    class kernel;
    class map;
}


class kpair
{
public:
    kpair( raft::kernel &a, 
           raft::kernel &b,
           const bool split,
           const bool join );

    kpair( kpair &a,
           raft::kernel  &b,
           const bool split,
           const bool join );

    kpair( raft::kernel &a,
           kpair        &n,
           const bool   split,
           const bool   join );

    kpair( kpair &a,
           kpair &b,
           const bool split,
           const bool join );

    kpair( raft::kernel &a, raft::kernel &b );
   
    void setOoO() noexcept;

protected:
    kpair        *next          = nullptr;
    kpair        *head          = nullptr;
    raft::kernel *src           = nullptr;
    bool          has_src_name  = false;
    std::string   src_name      = "";
    raft::kernel *dst           = nullptr;
    bool          has_dst_name  = false;
    std::string   dst_name      = "";
   
    
    bool          split_to      = false;
    core_id_t     src_out_count = 0;
    bool          join_from     = false;
    core_id_t     dst_in_count  = 0;

    bool          out_of_order  = false;

    friend class raft::map;
    friend kpair& operator >= ( kpair &a, raft::kernel &&b );
    friend kpair& operator >= ( kpair &a, raft::kernel &b );
    friend kpair& operator <= ( raft::kernel &a, raft::kernel  &b );
    friend kpair& operator <= ( raft::kernel &&a, raft::kernel &&b );
    friend kpair& operator <= ( raft::kernel &a,  kpair &b );
    friend kpair& operator <= ( raft::kernel &&a, kpair &b );
    friend kpair& operator >= ( kpair &a, kpair &b );
    friend kpair& operator >= ( raft::kernel &a, kpair &b );
    friend kpair& operator >= ( raft::kernel &&a, kpair &b );
};

//pre-declare these for "friend" of OoOkpair
class LOoOkpair;
class ROoOkpair;
/** 
 * XOoOkpair - simple wrapper for kernel so that the 
 * OoO syntax calls the proper operator overload for
 * the right shift operator. Otherise, this really
 * doesn't do much but reduce from kernel >> raft::order::out 
 * to kernel
 */
class LOoOkpair
{
public:
    constexpr LOoOkpair( raft::kernel &src ) : kernel( src ){};
    ~LOoOkpair() = default;
private:
    raft::kernel &kernel;
    
    friend kpair& operator >> ( LOoOkpair &a, raft::kernel &b );
    friend kpair& operator >> ( LOoOkpair &a, raft::kernel &&b );
};

class ROoOkpair 
{
public:
    constexpr ROoOkpair( kpair &p ) : other( p ){};
    ~ROoOkpair() = default;
private:
    kpair &other;
    friend kpair& operator >> ( ROoOkpair &a, raft::kernel &b );
    friend kpair& operator >> ( ROoOkpair &a, raft::kernel &&b );
};

kpair& operator >> ( raft::kernel &a,  raft::kernel &b  );
kpair& operator >> ( raft::kernel &&a, raft::kernel &&b );
kpair& operator >> ( kpair &a, raft::kernel &b );
kpair& operator >> ( kpair &a, raft::kernel &&b );

LOoOkpair& operator >> ( raft::kernel &a, const raft::order::spec &&order );
kpair&     operator >> ( LOoOkpair &a, raft::kernel &b );
kpair&     operator >> ( LOoOkpair &a, raft::kernel &&b );

ROoOkpair& operator >> ( kpair &a, const raft::order::spec &&order );
kpair&     operator >> ( ROoOkpair &a, raft::kernel &b );
kpair&     operator >> ( ROoOkpair &a, raft::kernel &&b );

kpair& operator <= ( raft::kernel &a, raft::kernel  &b );
kpair& operator <= ( raft::kernel &&a, raft::kernel &&b );
kpair& operator <= ( raft::kernel &a,  kpair &b );
kpair& operator <= ( raft::kernel &&a, kpair &b );

kpair& operator >= ( kpair &a, raft::kernel &b );
kpair& operator >= ( kpair &a, raft::kernel &&b );
kpair& operator >= ( kpair &a, kpair &b );

kpair& operator >= ( raft::kernel &a, kpair &b );
kpair& operator >= ( raft::kernel &&a, kpair &b );


kpair& operator <= ( raft::kernel &a, raft::basekset &&b );

#endif /* END _KPAIR_HPP_ */
