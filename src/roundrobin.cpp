#include "roundrobin.hpp"

roundrobin::roundrobin() : splitmethod()
{
}

roundrobin::~roundrobin()
{
}

FIFO*  
roundrobin::select_fifo( Port &port_list, const functype type )
{
   for( ;; )
   {
      for( auto &port : port_list )
      {
         /** 
          * TODO, big assumption here is that 
          * eventually a port will have space 
          */
         if( type == sendtype && port.space_avail() > 0 )
         {
            return( &( port ) );
         }
         if( type == gettype && port.size() > 0 )
         {
            return( &( port ) );
         }
      }
   }
}
