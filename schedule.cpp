#include "kernel.hpp"
#include "map.hpp"
#include "schedule.hpp"

Schedule::Schedule( Map &map )
{
   for( Kernel *kern : map.all_kernels )
   {
      (this)->scheduleKernel( kern );
   }
}

Schedule::~Schedule()
{
   /** nothing to do at the moment **/
}

bool
Schedule::scheduleKernel( Kernel *kernel )
{
   /** does nothing **/
   return( false );
}
