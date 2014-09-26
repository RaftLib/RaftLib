/**
 * monitor.hpp - 
 * @author: Jonathan Beard
 * @version: Tue Aug  5 13:22:27 2014
 * 
 * Copyright 2014 Jonathan Beard
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef _MONITOR_HPP_
#define _MONITOR_HPP_  1
#include <cstdint>
#include <cstring>
#include <cmath>

#include "Clock.hpp"
#include "ringbuffertypes.hpp"

#define NUMFRAMES    5
#define CONVERGENCE  .05

extern Clock *system_clock;

namespace Monitor
{
   /**
    * arrival_stats - contains to total item count and
    * frame count.
    */
   struct rate_stat
   {
      rate_stat() : items( 0 ),
                    frame_count( 0 )
      {}

      std::int64_t      items;
      std::int64_t      frame_count;
   };

   struct queue_stat
   {
      queue_stat() : items( 0 ),
                     frame_count( 0 )
      {
      }

      std::int64_t      items;
      std::int64_t      frame_count;
   };



   struct QueueData 
   {
      enum Units : std::size_t { Bytes = 0, KB, MB, GB, TB, N };
      const static std::array< double, Units::N > unit_conversion;
      const static std::array< std::string, N > unit_prints;
      /**
       * QueueData - basic constructor
       * @param sample_frequency - in seconds
       * @param nbytes - number of bytes in each queue item
       */
      QueueData( size_t nbytes ) : item_unit( nbytes )
      {
      }


      static double get_arrival_rate( volatile QueueData &qd , 
                                      Units unit )
      {
         
         if( qd.arrival.items == 0 )
         {
            return( 0.0 );
         }
         return( ( (((double)qd.arrival.items ) * qd.item_unit) / 
                     (qd.resolution.curr_frame_width * 
                        (double)qd.arrival.frame_count ) ) * 
                           (double)unit_conversion[ unit ] );
      }

      static double get_departure_rate( volatile QueueData &qd, 
                                        Units unit )
      {
         if( qd.departure.items == 0 )
         {
            return( 0.0 );
         }
         return( ( (((double)qd.departure.items ) * qd.item_unit ) / 
                     (qd.resolution.curr_frame_width * 
                        (double)qd.departure.frame_count ) ) * 
                           (double)unit_conversion[ unit ] );
      }

      static double get_mean_queue_occupancy( volatile QueueData &qd ) 
      {
         if( qd.mean_occupancy.items == 0 )
         {
            return( 0 );
         }
         return( qd.mean_occupancy.items / qd.mean_occupancy.frame_count );
      }

      static double get_utilization( volatile QueueData &qd )
      {
         const auto denom( 
            QueueData::get_departure_rate( qd, Units::Bytes ) );
         if( denom == 0 )
         {
            return( 0.0 );
         }
         return( 
            QueueData::get_arrival_rate( qd, Units::Bytes ) / 
               denom );
      }

      static std::ostream& print( volatile QueueData &qd, 
                                  Units unit,
                                  std::ostream &stream,
                                  bool csv = false )
      {
         if( ! csv )
         {
            stream << "Arrival Rate: " << 
               QueueData::get_arrival_rate( qd, unit ) << " " << 
                  QueueData::unit_prints[ unit ] << "/s" << "\n";
            stream << "Departure Rate: " << 
               QueueData::get_departure_rate( qd, unit ) << " " << 
                  QueueData::unit_prints[ unit ] << "/s" << "\n";
            stream << "Mean Queue Occupancy: " << 
               QueueData::get_mean_queue_occupancy( qd ) << "\n";
            stream << "Utilization: " << 
               QueueData::get_utilization( qd );
         }
         else
         {
               stream << QueueData::get_arrival_rate( qd, unit ) << ","; 
               stream << QueueData::get_departure_rate( qd, unit ) << ","; 
               stream << QueueData::get_mean_queue_occupancy( qd ) << ",";
               stream << QueueData::get_utilization( qd );

         }
         return( stream );
      }

      rate_stat              arrival;
      rate_stat              departure;
      queue_stat             mean_occupancy;
      frame_resolution       resolution;

      const size_t           item_unit;
   };
}

const std::array< double, 
                  Monitor::QueueData::Units::N > 
                  Monitor::QueueData::unit_conversion
                      = {{ 1              /** bytes **/,
                           0.000976562    /** kilobytes **/,
                           9.53674e-7     /** megabytes **/, 
                           9.31323e-10    /** gigabytes **/,
                           9.09495e-13    /** terabytes **/ }};

const std::array< std::string, 
                  Monitor::QueueData::Units::N > 
                     Monitor::QueueData::unit_prints
                         = {{ "Bytes", "KB", "MB", "GB", "TB" }};
#endif /* END _MONITOR_HPP_ */
