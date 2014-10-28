#include "roundrobin.hpp"

roundrobin::roundrobin() : splitmethod()
{
}

roundrobin::~roundrobin()
{
}

FIFO*  
roundrobin::select_fifo( Port &port_list )
{
   for( ;; )
   {
      for( auto &port : port_list )
      {
         /** 
          * TODO, big assumption here is that 
          * eventually a port will have space 
          */
         if( port.space_avail() > 0 )
         {
            return( &( port ) );
         }
      }
   }
}
