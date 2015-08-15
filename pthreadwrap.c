/**
 * pthreadwrap.c 
 * @author: Jonathan Beard
 * @version: Sat Aug 15 15:37:47 2015
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
#include "pthreadwrap.h"
#include <stdio.h>
#include <stdlib.h>
#include <sched.h>

inline int pthread_mutex_lock_d( pthread_mutex_t *m,
                                  const char *FILENAME,
                                  const int  LINENO )
{
#ifdef THREADDEBUG
   while( pthread_mutex_trylock( m ) != 0 )
   {
      fprintf( stderr, "Spinning from file name ( %s ) at line no ( %d )\n",
         FILENAME,
         LINENO );
       sched_yield();
   }
#else
   pthread_mutex_lock( m );
#endif
   return( 0 );
}
