/**
 * procwait.hpp - 
 * @author: Jonathan Beard
 * @version: Wed Aug 28 17:32:47 2013
 */
#ifndef _PROCWAIT_HPP_
#define _PROCWAIT_HPP_  1

#include <cstdint>
#include <vector>
#include <unistd.h>

class ProcWait{
public:
   ProcWait( const int64_t num_processes );

   virtual ~ProcWait();

   /* AddProcess - register a process to wait for */
   void AddProcess( pid_t process );
   /* WaitForChildren - blocks until all children are
    * fully exited
    */
   void WaitForChildren();

private:
   std::vector< pid_t > proc_list;
};

#endif /* END _PROCWAIT_HPP_ */
