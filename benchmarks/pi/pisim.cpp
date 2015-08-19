/**
 * pisim.cpp - simple but inneficient way to calculate pi,
 * does make a fun topology.
 * @author: Jonathan Beard
 * @version: Tue Jul 28 21:37:34 2015
 * 
 * Copyright 2015 Jonathan Beard
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
#include <cstdlib>
#include <cstddef>
#include <iostream>
#include <functional>
#include <raft>
#include <raftio>
#include <raftrandom>
#include <cmath>
/** command args package **/
#include <cmd>

template < typename  T > class pisim : public raft::kernel
{
public:
   pisim() : kernel()
   {
      input.addPort< T >( "x"  );
      input.addPort< T >( "y"  );
      output.addPort< T >( "pi" );
   }

   virtual ~pisim(){};

   virtual raft::kstatus run()
   {
      T x;
      T y;
      input[ "x" ].pop( x );
      input[ "y" ].pop( y );
      if( std::islessequal( ( x * x ) + ( y * y ),  static_cast< T >( 1 ) ) )
      {
         successes++;
         const T out( 4 * ( static_cast< T >( successes ) / 
                               static_cast< T >( throws ) ) );
         output[ "pi" ].push( out );
      }
      throws++;
      return( raft::proceed );
   }

private:
   std::uintmax_t throws    = 0;
   std::uintmax_t successes = 0;
};

int
main( int argc, char **argv )
{
   bool         help( false );
   std::int64_t count( 1000 );
   using sim_t  = double;
   CmdArgs cmdargs( argv[ 0 ], std::cout, std::cerr );
   cmdargs.addOption( new Option< bool >( help,
                                          "-h",
                                          "print this menu and exit" ) );
   cmdargs.addOption( new Option< std::int64_t >( count,
                                                  "-count",
                                                  "number of simulation iterations" ) );
   cmdargs.processArgs( argc, argv );
   if( help )
   {
      cmdargs.printArgs();
      exit( EXIT_SUCCESS );
   }
                                                  
   using rndgen = raft::random_variate< sim_t, raft::uniform >;
   using sim    = pisim< sim_t >;
   using print  = raft::print< sim_t, '\n' >;
   auto kernels(
      raft::map.link( raft::kernel::make< rndgen >( 0.0, 1.0, count ), 
                      raft::kernel::make< sim >(), "x" )
   );
   raft::map.link( raft::kernel::make< rndgen >( 0.0, 1.0, count ),
                   &kernels.getDst(), "y" );

   std::ofstream ofs( "/dev/null" );
   raft::map.link( &kernels.getDst(),
                   raft::kernel::make< print >( ofs ) ); 
   raft::map.exe();
   ofs.close();
   return( EXIT_SUCCESS );
}
