/**
 * procwait.cpp - 
 * @author: Jonathan Beard
 * @version: Wed Aug 28 17:32:47 2013
 */
#include "procwait.hpp"

#include <cstdio>
#include <cstdlib>
#include <sched.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <cassert>
#include <errno.h>
#include <cinttypes>
#include <iostream>
   
   
ProcWait::ProcWait( const int64_t num_processes )
                                    : proc_list( num_processes )
{
   /* nothing really to do */
}

ProcWait::~ProcWait()
{
   /* nothing to do */
}

void 
ProcWait::AddProcess( pid_t process )
{
   assert( process > 0 );
   proc_list.push_back( process );
}

void 
ProcWait::WaitForChildren()
{
   for( pid_t proc : proc_list )
   {
      int status( 0 );
      errno = 0;
      if( waitpid( proc, &status, WUNTRACED ) == -1 )
      {
      /* check return status */
      if( ! WIFEXITED( status ) )
      {
         perror( "Failed to wait for child process!!" );
         if( errno != ECHILD ) /* already exited normally */
         {
            std::cerr << "Child process (" << proc << ") failed "
            << "to exit normally!!\n";
         }
      }
      }
   }
}
