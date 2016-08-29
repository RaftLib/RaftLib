/**
 * gatekeeper.hpp - 
 * @author: Jonathan Beard
 * @version: Thu Aug 29 09:17:03 2013
 */
#ifndef _GATEKEEPER_HPP_
#define _GATEKEEPER_HPP_  1



#include <cstdint>
#include <map>
#include <string>

class Gate;

class GateKeeper{
public:
   /**
    * GateKeeper - needs to be initialized with the 
    * number of processes that we're gating.
    * @param number_of_processes - total number you expect
    */
   GateKeeper( const int64_t number_of_processes );

   /** destructor **/
   virtual ~GateKeeper();

   /**
    * add a new gate if you need one, the object starts
    * off with no gates so you better add some if
    * you want to use it.
    */
   bool  RegisterGate( const std::string gate_name );

   /**
    * wait for all other processes to get to this gate 
    */
   bool  WaitForGate( const std::string gate_name );
  
   /**
    * rest the gate at name, returns false if no gate exists
    */
   bool  ResetGate( const std::string gate_name );
  

   void  ResetAllGates();

   /**
    * let the object know you've forked and all ptrs
    * need to be re-initialized 
    */
   void  HandleFork();

   
private:

   bool GateExists( const std::string name );

   std::map< std::string, Gate > gates;
   const int64_t num_processors;
};

#endif /* END _GATEKEEPER_HPP_ */
