#include <iostream>

#include "kernel.hpp"
#include "map.hpp"
#include "schedule.hpp"


Schedule::Schedule( Map &map ) : map_ref( map )
{
   handlers.addHandler( raft::quit, Schedule::quitHandler ); 
}

Schedule::~Schedule()
{
   /** nothing to do at the moment **/
}

void
Schedule::init()
{
   for( raft::kernel *kern : map_ref.all_kernels )
   {
      (this)->scheduleKernel( kern );
   }
}


void
Schedule::quitHandler( FIFO              &fifo, 
                       raft::kernel      *kernel,
                       const raft::signal signal,
                       void              *data )
{
   /**
    * NOTE: This should be the only action needed
    * currently, however that may change in the futre
    * with more features and systems added.
    */
   fifo.invalidate();   
}

void 
Schedule::checkSystemSignal( raft::kernel * const kernel, 
                             void *data,
                             SystemSignalHandler &handlers )
{
   auto &input_ports( kernel->input );
   for( auto &port : input_ports )
   {
      const auto curr_signal( port.signal_peek() );
      if( __builtin_expect(
         ( curr_signal > 0 && curr_signal < raft::MAX_SYSTEM_SIGNAL ),
         0 ))
      {
         handlers.callHandler( curr_signal,
                               port,
                               kernel,
                               data );
      }
   }
}

bool
Schedule::scheduleKernel( raft::kernel *kernel )
{
   /** does nothing **/
   return( false );
}

void 
Schedule::sendEndOfData( raft::kernel *kernel,
                         void         *data )
{
   auto &output_ports( kernel->output );
   for( auto &port : output_ports )
   {
      port.inline_signal_send( raft::quit ); 
   }
}


bool
Schedule::kernelHasInputData( raft::kernel *kernel )
{
   auto &port_list( kernel->input );
   if( ! port_list.hasPorts() )
   {
      /** only output ports, keep calling till exits **/
      return( true );
   }
   for( auto &port : port_list )
   {
      if( port.size() )
      {
         return( true );
      }
   }
   return( false );
}



bool
Schedule::kernelHasNoInputPorts( raft::kernel *kernel )
{
   auto &port_list( kernel->input );
   /** assume data check is already complete **/
   for( auto &port : port_list )
   {
      if( ! port.is_invalid() )
      {
         return( false );
      }
   }
   return( true );
}
