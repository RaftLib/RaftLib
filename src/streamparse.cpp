/**
 * streamparse.cpp - 
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
#include "streamparse.hpp"
#include "streamparseexception.tcc"

kpair& 
operator >> ( raft::kernel &a, raft::kernel &b )
{
    auto *ptr( new kpair( a, b ) );
    return( *ptr );
}

kpair&
operator >> ( raft::kernel_wrapper &&a, raft::kernel_wrapper &&b )
{
    auto *ptr( new kpair( a, b ) );
    return( *ptr );
}

kpair&
operator >> ( raft::kernel &a, raft::kernel_wrapper &&w )
{
    auto *ptr( new kpair( a, w) );
    return( *ptr );
}



kpair&  
operator >> ( kpair &a, raft::kernel &b )
{
    auto *ptr( new kpair( a, b, false, false ) );
    return( *ptr );
}

kpair&
operator >> ( kpair &a, raft::kernel_wrapper &&w )
{
    auto *ptr( new kpair( a, w, false, false ) );
    return( *ptr );
}

/**
 * >>, we're using the raft::order::spec as a linquistic tool
 * at this point. It's only used for disambiguating functions.
 */
LOoOkpair
operator >> ( raft::kernel &a, const raft::order::spec &&order )
{
    UNUSED( order );
    LOoOkpair out( a );
    return( out );
}

kpair&
operator >> ( const LOoOkpair &&a, raft::kernel &b )
{
    auto *ptr( 
        new kpair( a.value, 
                   b, 
                   false, 
                   false ) 
    );
    ptr->setOoO();
    return( *ptr );
}


kpair&
operator >> ( const LOoOkpair &&a, raft::kernel_wrapper &&w )
{
    auto *ptr( 
        new kpair( a.value, w, false, false ) 
    );
    ptr->setOoO();
    return( *ptr );
}

/**
 * >>, we're using the raft::order::spec as a linquistic tool
 * at this point. It's only used for disambiguating functions.
 */
ROoOkpair
operator >> ( kpair &a, const raft::order::spec &&order )
{
    ROoOkpair out( a );
    UNUSED( order );
    return( out );
}

kpair&
operator >> ( const ROoOkpair &&a, raft::kernel &b )
{
    auto * ptr(
        new kpair( a.value, b, false, false )
    );
    ptr->setOoO();
    return( *ptr );
}

kpair&
operator >> ( const ROoOkpair &&a, raft::kernel_wrapper &&w )
{
    auto * ptr(
        new kpair( a.value, w, false, false )
    );
    ptr->setOoO();
    return( *ptr );
}

kpair&
operator <= ( raft::kernel &a, raft::kernel &b )
{
    auto *ptr( new kpair( a, b, true, false ) );
    return( *ptr );
}

kpair&
operator <= ( raft::kernel_wrapper &&a, raft::kernel_wrapper &&b )
{
    auto *ptr( new kpair( a, b, true, false ) );
    return( *ptr );
}

kpair&
operator <= ( raft::kernel &a, kpair &b )
{
    auto *ptr( new kpair( a, b, true, false ) );
    return( *ptr );
}

kpair&
operator <= ( raft::kernel_wrapper &&w, kpair &b )
{
    auto *ptr( new kpair( w, b, true, false ) );
    return( *ptr );
}

kpair& 
operator <= ( kpair &a, raft::kernel &b )
{
    auto *ptr( new kpair( a, b, true, false ) );
    return( *ptr );
}

kpair&
operator >= ( kpair &a, raft::kernel_wrapper &&w )
{
    auto *ptr( new kpair( a, w, false, true ) );
    return( *ptr );
}

kpair&
operator >= ( kpair &a, raft::kernel &b )
{
    auto *ptr( new kpair( a, b, false, true ) );
    return( *ptr );
}

kpair&
operator >= ( kpair &a, kpair &b )
{
    auto *ptr( new kpair( a, b, false, true ) );
    return(*ptr);
}

kpair& 
operator >= ( raft::kernel &a, kpair &b )
{
    auto *ptr( new kpair( a, b, false, true ) );
    return(*ptr);
}

kpair& 
operator >= ( raft::kernel_wrapper &&w, kpair &b )
{
    auto *ptr( new kpair( w, b, false, true ) );
    return(*ptr);
}

kpair& operator <= ( raft::kernel &a, raft::basekset &&b )
{
    UNUSED( a );
    UNUSED( b );
    kpair *out( nullptr );
    return(*out );
}

kpair& operator >> ( raft::basekset &&a, raft::kernel &b )
{
    UNUSED( a );
    UNUSED( b );
    kpair *out( nullptr );
    return(*out);
}

kpair& operator >> ( raft::basekset &&a, raft::basekset &&b )
{
    UNUSED( a );
    UNUSED( b );
    kpair *out( nullptr );
    return( *out );
}
kpair& operator >= ( raft::basekset &&a, raft::kernel &b )
{
    UNUSED( a );
    UNUSED( b );
    kpair *out( nullptr );
    return( *out );
}

kpair& operator >= ( raft::basekset &&a, kpair &b )
{
    UNUSED( a );
    UNUSED( b );
    kpair *out( nullptr );
    return( *out );
}

ManipVecKern operator >> ( raft::kernel &a, raft::manip_vec_t &&b )
{
    return( ManipVecKern( a, b ) );
}

ManipVecPair operator >> ( kpair &a, raft::manip_vec_t &&b )
{    
    return( ManipVecPair( a, b ) );
}


/**
 * TODO, for both of the below, we can add more specific exceptions
 * that will tell the user if they're doing something really stupid
 */
kpair& operator >> ( const ManipVecKern &&a, raft::manip_vec_t &&b )
{
    throw NonsenseChainRaftManipException( 
        "multiple stream parse manipulators should be combined as in (raft::manip< [manip 1], [manip 2] >::value" );
    UNUSED( a );
    UNUSED( b );
    kpair *out( nullptr );
    return( *out );
}
kpair& operator >> ( const ManipVecPair &&a, raft::manip_vec_t &&b )
{
    throw NonsenseChainRaftManipException( 
        "multiple stream parse manipulators should be combined as in (raft::manip< [manip 1], [manip 2] >::value" );
    UNUSED( a );
    UNUSED( b );
    kpair *out( nullptr );
    return( *out );
}

