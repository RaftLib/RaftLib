/**
 * streamparse.hpp - 
 * @author: Jonathan Beard
 * @version: Thu Dec 29 05:52:20 2016
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
#ifndef _STREAMPARSE_HPP_
#define _STREAMPARSE_HPP_  1

#include "kpair.hpp"

kpair& operator >> ( raft::kernel &a,  raft::kernel &b  );
kpair& operator >> ( raft::kernel_wrapper &&a, raft::kernel_wrapper &&b );
kpair& operator >> ( raft::kernel &a, raft::kernel_wrapper &&w );

kpair& operator >> ( kpair &a, raft::kernel &b );
kpair& operator >> ( kpair &a, raft::kernel_wrapper &&w );

LOoOkpair& operator >> ( raft::kernel &a, const raft::order::spec &&order );
kpair&     operator >> ( LOoOkpair &a, raft::kernel &b );
kpair&     operator >> ( LOoOkpair &a, raft::kernel_wrapper &&w );

ROoOkpair& operator >> ( kpair &a, const raft::order::spec &&order );
kpair&     operator >> ( ROoOkpair &a, raft::kernel &b );
kpair&     operator >> ( ROoOkpair &a, raft::kernel_wrapper &&w );


kpair& operator <= ( raft::kernel &a, raft::kernel  &b );
kpair& operator <= ( raft::kernel_wrapper &&a, raft::kernel_wrapper &&b );
kpair& operator <= ( raft::kernel &a,  kpair &b );
kpair& operator <= ( raft::kernel_wrapper &&w, kpair &b );
kpair& operator <= ( kpair &a, raft::kernel &b );


kpair& operator >= ( kpair &a, raft::kernel &b );
kpair& operator >= ( kpair &a, raft::kernel_wrapper &&w );
kpair& operator >= ( kpair &a, kpair &b );

kpair& operator >= ( raft::kernel &a, kpair &b );
kpair& operator >= ( raft::kernel_wrapper &&w, kpair &b );


kpair& operator <= ( raft::kernel &a, raft::basekset &&b );
kpair& operator >> ( RParaPair &a, const raft::parallel::type &&type );

kpair& operator >> ( raft::basekset &&a, raft::kernel &b );
kpair& operator >> ( raft::basekset &&a, raft::basekset &&b );
/**
 * this case looks like this:
 * m += raft::kset( g0, g1, g2 ) >= j;
 */
kpair& operator >= ( raft::basekset &&a, raft::kernel &b );
/**
 * this case looks like this:
 * m += raft::kset( g0, g1, g2 ) >= j >> print;
 * due to >> operator precendent in c++
 */
kpair& operator >= ( raft::basekset &&a, kpair &b ); 

#endif /* END _STREAMPARSE_HPP_ */
