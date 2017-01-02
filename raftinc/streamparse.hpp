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

namespace raft
{
    class kernel;
    class kernel_wrapper;
}


template < class T, int N > struct Wrapper
{
    constexpr Wrapper( T &t ) : value( t ){};
    
    virtual ~Wrapper() = default;

    T &value;
};

template < class T, class K, int N > struct WrapperPlusConstant : Wrapper< T, N >
{
    constexpr WrapperPlusConstant( T &t, const K sp ) : Wrapper< T, N >( t ),
                                                        sp( sp ){};
    const K sp;
};



using LOoOkpair     = Wrapper<  raft::kernel, 
                                0 >; 
using ROoOkpair     = Wrapper<  kpair, 
                                1 >;
using ManipVecPair  = WrapperPlusConstant<  kpair, 
                                            raft::manip_vec_t, 
                                            2 >;
using ManipVecKern  = WrapperPlusConstant<  raft::kernel, 
                                            raft::manip_vec_t , 
                                            3 >;

kpair& operator >> ( raft::kernel &a,  raft::kernel &b  );
kpair& operator >> ( raft::kernel_wrapper &&a, raft::kernel_wrapper &&b );
kpair& operator >> ( raft::kernel &a, raft::kernel_wrapper &&w );

kpair& operator >> ( kpair &a, raft::kernel &b );
kpair& operator >> ( kpair &a, raft::kernel_wrapper &&w );

LOoOkpair operator >> ( raft::kernel &a, const raft::order::spec &&order );
kpair&     operator >> ( const LOoOkpair &&a, raft::kernel &b );
kpair&     operator >> ( const LOoOkpair &&a, raft::kernel_wrapper &&w );

ROoOkpair operator >> ( kpair &a, const raft::order::spec &&order );
kpair&     operator >> ( const ROoOkpair &&a, raft::kernel &b );
kpair&     operator >> ( const ROoOkpair &&a, raft::kernel_wrapper &&w );


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

/**
 * raft::kernel >> manip_vec_t (bare) 
 */
ManipVecKern operator >> ( raft::kernel &a, raft::manip_vec_t &&b );

/**
 * kpair >> manip_vec_t (bare) 
 */
ManipVecPair operator >> ( kpair &a, raft::manip_vec_t &&b );

/** 
 * TODO, need to duplicate states for ManipVecKern or 
 * figure out a clever way not to have to...
 */



/**
 * BEGIN ERROR STATES FOR PARSE 
 **/


/**
 * error state: #4 (for test cases)
 * manip_vec_t >=  raft::kernel 
 * NOTE: you can have a <= or >= after the bare op just not before
 */
kpair& operator >= ( const ManipVecKern  &&a, const raft::kernel &b  );

/** error state: #4a (for test cases) **/
kpair& operator >= ( const ManipVecKern  &&a, kpair &b  );

/** error state: #4b (for test cases) **/
kpair& operator >= ( const raft::manip_vec_t &&a, kpair &b  );
/** error state: #4c (for test cases) **/
kpair& operator >= ( const raft::manip_vec_t &&a, const raft::kernel &b  );


/**
 * error state: #3 (for test cases)
 * raft::kernel <=  manip_vec_t (bare) 
 * NOTE: you can have a <= or >= after the bare op just not before
 */
kpair& operator <= ( const raft::kernel &a, const raft::manip_vec_t &&b );

/** error state: #3a (for test cases) **/
kpair& operator <= ( kpair &a,  const raft::manip_vec_t &&b );


/** 
 * errorstate: #2 (for test cases)
 * raft::manip_vec_tl >> raft::manip_vec_t
 */ 
kpair& operator >> ( const ManipVecKern &&a, const raft::manip_vec_t &&b );

/**
 * error state: #1 (for test cases)
 * raft::manip_vec_tl >> raft::manip_vec_t
 */
kpair& operator >> ( const ManipVecPair &&a, const raft::manip_vec_t &&b );

/** 
 * error state: #5 (for test cases )
 * raft::manip_vec_tl <= raft::manip_vec_t
 */
//TODO, not a good way to catch this one without some extremely fancy templates
/** 
 * error state: #5 (for test cases )
 * raft::manip_vec_tl => raft::manip_vec_t
 */
//TODO, not a good way to catch this one without some extremely fancy templates

#endif /* END _STREAMPARSE_HPP_ */
