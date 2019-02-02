/**
 *
 * @author: Jonathan Beard
 * @version: Sat Feb 27 19:10:26 2016
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
#include <cstdint>
#include <cstdio>
#include <cstddef>
#include <raft>
#include <raftmanip>
#include <cstdlib>
#include <cassert>

#include "streamManipUtils.h"


int
main()
{
    start   s;
    middle  m, m2;
    last    l;

    raft::map M;
    M += s  >= ( raft::manip< raft::parallel::process  >::value >> m2 >> l ); 
    /** let exception fall through and error to test **/
    return( EXIT_FAILURE );
}
