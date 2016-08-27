/**
 * gate.hpp - 
 * @author: Jonathan Beard
 * @version: Wed Aug 28 17:32:47 2013
 */
#ifndef _GATE_HPP_
#define _GATE_HPP_  1

#include <cstdint>
#include <ostream>
#include <semaphore.h>

class Gate{
public:   
   /**
    * Gate - constructor.  Takes in a process
    * count, which should be the number of processes
    * using this gate object
    * @param proc_count
    */
   Gate( int64_t proc_count );
   /**
    * Copy constructor - copies the shm_key, sem_key and
    * pointers to already open shm and semaphores.  This
    * has the added side-effect that these two gate objects
    * now have the exact same semaphore and sync which means
    * that you should really use one or the other and not 
    * both.
    * @param   g  - const Gate&
    */
   Gate( const Gate &g );

   /** destructor, doesn't do much */
   virtual ~Gate();

   /** 
    * ReOpen - re-initializes the ptrs after forking, should
    * only be called once and immediately after forking.
    */
   void ReOpen();
   
   /**
    * Barrier - as the name suggests it blocks until all other
    * processes have reached this barrier.
    */
   void Barrier();

   /**
    * Reset - blocks until all processes that share this barrier
    * have reached it and they are in fact reset to initial 
    * conditions.  Should be called if you want to use this
    * barrier over and over again as in a loop 
    */
   void Reset();

   /**
    * Destroy - As the name suggests, this method performs the
    * cleanup for the Gate object.  It unmaps SHM, closes all
    * file handles.
    */
   void Destroy();

   /**
    * print - just in case you need to debug something, this 
    * will print all pertinent information to the stream.  
    * Things like: # processes, is it called from a child process,
    * the SHM key and the semaphore key
    * @param   stream - std::ostream&
    * @return  std::ostream&
    */
   std::ostream& print( std::ostream &stream );


private:
   
   static const int buffer_size = 40;
   
   typedef int64_t proc_t;

   volatile proc_t* proc;
   sem_t   *procsem;

   char   shm_key[ buffer_size ];
   char   sem_key[ buffer_size ];
   const  proc_t   process_count;
   bool   child;
};
#endif /* END _GATE_HPP_ */
