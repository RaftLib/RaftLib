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
   for( Kernel *kern : map_ref.all_kernels )
   {
      (this)->scheduleKernel( kern );
   }
}

bool
Schedule::scheduleKernel( Kernel *kernel )
{
   /** does nothing **/
   return( false );
}
