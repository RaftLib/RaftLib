#include <iostream>

#include "kernel.hpp"
#include "map.hpp"
#include "schedule.hpp"
#include "optdef.hpp"


Schedule::Schedule( raft::map &map ) : kernel_set( map.all_kernels ),
                                 source_kernels( map.source_kernels ),
                                 dst_kernels( map.dst_kernels )
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
   /** default, do nothing **/
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

   (void) kernel;
   (void) signal;
   (void) data;

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

void
Schedule::scheduleKernel( raft::kernel * const kernel )
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
   handleSchedule( kernel );
   return;
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
      if( port.size() > 0 )
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


bool
Schedule::kernelRun( raft::kernel * const kernel,
                     volatile bool       &finished,
                     jmp_buf             *gotostate,
                     jmp_buf             *kernel_state )
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
            void *ptr = reinterpret_cast< void* >( (*in_begin).first );
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
            void *ptr = reinterpret_cast< void* >( (*in_begin).first );
            (*in_begin).second( ptr );
            ++in_begin;
    }
    in->clear();
    out->clear();
    peekset->clear();
    return;
}
