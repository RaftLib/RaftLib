#include <iostream>

#include "kernel.hpp"
#include "map.hpp"
#include "schedule.hpp"
#include "optdef.hpp"


Schedule::Schedule( Map &map ) : map_ref( map )
{
   //TODO, see if we want to keep this 
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


raft::kstatus
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
   return( raft::stop );
}

void 
Schedule::invalidateOutputPorts( raft::kernel *kernel )
{
   
   auto &output_ports( kernel->output );
   for( auto &port : output_ports )
   {  
      port.invalidate();
   }
   return;
}

raft::kstatus
Schedule::checkSystemSignal( raft::kernel * const kernel, 
                             void *data,
                             SystemSignalHandler &handlers )
{
   auto &input_ports( kernel->input );
   raft::kstatus ret_signal( raft::proceed );
   for( auto &port : input_ports )
   {
      if( port.size() == 0 )
      {
         continue;
      }
      const auto curr_signal( port.signal_peek() );
      if( __builtin_expect(
         ( curr_signal > 0 && curr_signal < raft::MAX_SYSTEM_SIGNAL ),
         0 ))
      {
         port.signal_pop();
         /**
          * TODO, right now there is special behavior for term signal only, 
          * what should we do with others?  Need to decide that.
          */
         
         if( handlers.callHandler( curr_signal,
                               port,
                               kernel,
                               data ) == raft::stop )
         {
            ret_signal = raft::stop;
         }
      }
   }
   return( ret_signal );
}

bool
Schedule::scheduleKernel( raft::kernel *kernel )
{
   /** does nothing **/
   return( false );
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
      if( likely( port.size() > 0 ) )
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
      if( likely( ! port.is_invalid() ) )
      {
         return( false );
      }
   }
   return( true );
}

bool
Schedule::kernelRun( raft::kernel * const kernel,
                      volatile bool       &finished,
                      jmp_buf             *gotostate,
                      jmp_buf             *kernel_state )
{
   if( kernelHasInputData( kernel ) )
   {
      const auto sig_status = kernel->run();
      if( sig_status == raft::stop )
      {
         invalidateOutputPorts( kernel );
         finished = true;
      }
   } 
   /** 
    * must recheck data items again after port valid check, there could
    * have been a push between these two conditional statements.
    */
   else if(  kernelHasNoInputPorts( kernel ) && ! kernelHasInputData( kernel ) )
   {
      invalidateOutputPorts( kernel );
      finished = true;
   }
   return( true );
}
