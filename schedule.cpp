#include <iostream>

#include "kernel.hpp"
#include "map.hpp"
#include "schedule.hpp"


Schedule::Schedule( Map &map ) : map_ref( map )
{
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

bool
Schedule::scheduleKernel( raft::kernel *kernel )
{
   /** does nothing **/
   return( false );
}

void
Schedule::invalidateOutputPorts( raft::kernel *kernel )
{
   for( auto &port : kernel->output )
   {
      port.invalidate();
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
