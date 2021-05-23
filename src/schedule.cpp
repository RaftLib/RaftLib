#include <iostream>

#include "kernel.hpp"
#include "map.hpp"
#include "schedule.hpp"
#include "defs.hpp"
#include "sysschedutil.hpp"


Schedule::Schedule( raft::map &map ) :  kernel_set( map.all_kernels ),
                                        source_kernels( map.source_kernels ),
                                        dst_kernels( map.dst_kernels ),
                                        internally_created_kernels( map.internally_created_kernels )
{
}

Schedule::~Schedule()
{
}


void
Schedule::init()
{
   /** default, do nothing **/
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


void
Schedule::revalidateOutputPorts( raft::kernel *kernel )
{

   auto &output_ports( kernel->output );
   for( auto &port : output_ports )
   {
      port.revalidate();
   }
   return;
}



/**
 * NOTE: if you add code here, make sure you go back to the
 * "updateKernels" function in the mapbase.hpp file to update
 * that code as well, much is similar to the code there, it's
 * basically what is called at setup, this function below
 * is used dynamically. Both share the same kernelkeeper
 * objects which are thread safe to add to and remove from.
 */
void
Schedule::schedule_kernel( raft::kernel * const kernel )
{
   /**
    * NOTE: The kernel param should be ready to rock,
    * we just need to add it here.  The data structures
    * xx_kernels all expect a fully ready kernel, well
    * the threads monitoring the system do at least and
    * this is one.  What we need to do to kick off the
    * execution is add it to the handleSchedule virtual
    * function which is implemented by each scheduler.
    */
   assert( kernel != nullptr );
   if( ! kernel->input.hasPorts() )
   {
      source_kernels +=  kernel;
   }
   if( ! kernel->output.hasPorts() )
   {
      dst_kernels += kernel;
   }
   kernel_set += kernel;
   if( kernel->internal_alloc )
   {
      internally_created_kernels += kernel;
   }
   handleSchedule( kernel );
   return;
}
 

bool 
Schedule::terminus_complete()
{
    /**
     * we might need to re-evaluate this if there
     * are no termini in the graph (e.g., there are
     * cycles, right now though, we can have interior
     * cycles only so this is likely fine, but...
     * we'll have to come  up with a better way in the
     * future. 
     */
    return( complete );
}
    
/**
 * reset_streams - reset all streams within the defined
 * graph so that they're in a state where they can be 
 * re-used, basically undoing the logic that is used
 * for shutting down the dataflow graph when no data
 * is coming. 
 * @return void. 
 */
void 
Schedule::reset_streams()
{
    auto &kernels( kernel_set.acquire() );
    for( raft::kernel * const k : kernels )
    {
        revalidateOutputPorts( k ); 
    }
    kernel_set.release();
    complete = false;
    return;
}

void
Schedule::signal_complete()
{
    complete = true; 
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
    /**
     * NOTE: this was added as a reqeuest, need to update wiki,
     * the first hit to this one will take an extra few cycles
     * to process the jmp, however, after that, the branch
     * taken is incredibly easy and we should be able to do 
     * this as if the switch statement wasn't there at all. 
     * - an alternative to using the kernel variable would
     * be to implement a new subclass of kernel...that's doable
     * too but we'd have to make dependent template functions
     * that would use the type info to select the right behavior
     * which we're doing dynamically below in the switch statement.
     */
    switch( kernel->sched_behav )
    {
        case( raft::any_port ):
        {
            for( auto &port : port_list )
            {
               const auto size( port.size() );
               if( size > 0 )
               {
                  return( true );
               }
            }
        }
        break;
        case( raft::all_port ):
        {
            for( auto &port : port_list )
            {
               const auto size( port.size() );
               /** no data avail on this port, return false **/
               if( size == 0 )
               {
                  return( false );
               }
            }
            /** all ports have data, return true **/
            return( true );
        }
        break;
        default:
        {
            //TODO add exception class here
            std::cerr << "invalid scheduling behavior set, exiting!\n";
            exit( EXIT_FAILURE );
        }
    }
    /** we should have returned before here, keep compiler happy **/
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


bool
Schedule::kernelRun( raft::kernel * const kernel,
                     volatile bool       &finished )
{

   if( kernelHasInputData( kernel ) )
   {
      const auto sig_status( kernel->run() );
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
   if(  kernelHasNoInputPorts( kernel ) && ! kernelHasInputData( kernel ) )
   {
      invalidateOutputPorts( kernel );
      finished = true;
   }
   return( true );
}

void
Schedule::setPtrSets( raft::kernel * const kernel,
                      ptr_map_t    * const in,
                      ptr_set_t    * const out,
                      ptr_set_t    * const peekset )
{
    assert( in  != nullptr );
    assert( out != nullptr );
    assert( peekset != nullptr );
    /**
     * looks a bit odd initially, but the same
     * peekset is set for each kernel's FIFO's
     * within the view of the kernel they'll be
     * accessed sequentially so no contention
     */
    for( auto &port : kernel->input )
    {
        port.setPtrMap( in );
        port.setInPeekSet( peekset );
    }
    for( auto &port : kernel->output )
    {
        port.setPtrSet( out );
        port.setOutPeekSet( peekset );
    }
    return;
}

void
Schedule::fifo_gc( ptr_map_t * const in,
                   ptr_set_t * const out,
                   ptr_set_t * const peekset )
{
    auto in_begin( in->begin() ), in_end( in->end() );
    auto out_beg( out->begin() ), out_end( out->end() );
    while( out_beg != out_end )
    {
        if( (*out_beg) != (*in_begin).first )
        {
            void * const ptr = reinterpret_cast< void* >( (*in_begin).first );
            (*in_begin).second( ptr );
            ++in_begin;
        }
        else
        {
            ++out_beg;
            ++in_begin;
        }
    }
    while( in_begin != in_end )
    {
            void * const ptr = reinterpret_cast< void* >( (*in_begin).first );
            (*in_begin).second( ptr );
            ++in_begin;
    }
    in->clear();
    out->clear();
    peekset->clear();
    return;
}
