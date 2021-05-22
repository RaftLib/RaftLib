/**
 * generate.tcc - 
 * @author: Jonathan Beard
 * @version: Fri Dec  4 07:26:02 2015
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
#ifndef GENERATE_TCC
#define GENERATE_TCC  1
#include <random>
#include <string>
#include <cstdint>

namespace raft
{

namespace test
{

#ifndef STRING_NAMES
template < typename T > class generate : public raft::kernel
{
public:
   generate( std::int64_t count = 1000 ) : raft::kernel(),
                                         count( count )
   {
      output.addPort< T >( "number_stream"_port );
   }

   virtual raft::kstatus run()
   {
      if( count-- > 1 )
      {
         auto &ref( output[ "number_stream"_port ].template allocate< T >() );
         ref = static_cast< T >( (this)->count );
         output[ "number_stream"_port ].send();
         return( raft::proceed );
      }
      /** else **/
      auto &ref( output[ "number_stream"_port].template allocate< T >() );
      ref = static_cast< T >( (this)->count );
      output[ "number_stream"_port].send( raft::eof );
      return( raft::stop );
   }

private:
   std::int64_t count;
};


template <> class generate< std::string > : public raft::kernel
{
public:
   generate( std::int64_t count = 1000 ) : raft::kernel(),
                                           count( count ),
                                           gen( 15 ),
                                           distrib( 65, 90 )
   {
      output.addPort< std::string >( "number_stream"_port );
   }

   virtual raft::kstatus run()
   {
      char str[ 8 ];
      str[7]='\0';
      for( auto i( 0 ); i < 7; i++ )
      {
         str[ i ] = (char) distrib( gen );
      }
      if( count-- > 1 )
      {
         
         auto &ref( output[ "number_stream"_port ].template allocate< std::string >() );
         ref = static_cast< std::string >( str );
         output[ "number_stream"_port].send();
         return( raft::proceed );
      }
      /** else **/
      auto &ref( output[ "number_stream"_port ].template allocate< std::string >() );
      ref = static_cast< std::string >( str );
      output[ "number_stream"_port ].send( raft::eof );
      return( raft::stop );
   }

private:
   std::int64_t count;
   std::mt19937 gen;
   std::uniform_int_distribution<> distrib;
};

#else
template < typename T > class generate : public raft::kernel
{
public:
   generate( std::int64_t count = 1000 ) : raft::kernel(),
                                         count( count )
   {
      output.addPort< T >( "number_stream" );
   }

   virtual raft::kstatus run()
   {
      if( count-- > 1 )
      {
         auto &ref( output[ "number_stream" ].template allocate< T >() );
         ref = static_cast< T >( (this)->count );
         output[ "number_stream" ].send();
         return( raft::proceed );
      }
      /** else **/
      auto &ref( output[ "number_stream"].template allocate< T >() );
      ref = static_cast< T >( (this)->count );
      output[ "number_stream"].send( raft::eof );
      return( raft::stop );
   }

private:
   std::int64_t count;
};


template <> class generate< std::string > : public raft::kernel
{
public:
   generate( std::int64_t count = 1000 ) : raft::kernel(),
                                           count( count ),
                                           gen( 15 ),
                                           distrib( 65, 90 )
   {
      output.addPort< std::string >( "number_stream" );
   }

   virtual raft::kstatus run()
   {
      char str[ 8 ];
      str[7]='\0';
      for( auto i( 0 ); i < 7; i++ )
      {
         str[ i ] = (char) distrib( gen );
      }
      if( count-- > 1 )
      {
         
         auto &ref( output[ "number_stream" ].template allocate< std::string >() );
         ref = static_cast< std::string >( str );
         output[ "number_stream"].send();
         return( raft::proceed );
      }
      /** else **/
      auto &ref( output[ "number_stream" ].template allocate< std::string >() );
      ref = static_cast< std::string >( str );
      output[ "number_stream" ].send( raft::eof );
      return( raft::stop );
   }

private:
   std::int64_t count;
   std::mt19937 gen;
   std::uniform_int_distribution<> distrib;
};

#endif

} //end namespace test

} //end namespace raft
#endif /* END GENERATE_TCC */
