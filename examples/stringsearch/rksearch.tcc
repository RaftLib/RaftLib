/**
 * rksearch.tcc - 
 * @author: Jonathan Beard
 * @version: Sun Nov  9 17:08:56 2014
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
#ifndef _RKSEARCH_TCC_
#define _RKSEARCH_TCC_  1


template <> class search< rabinkarp > : public kernel
{
public:
   search( const std::string term ) : kernel()
   {
      m = term.length();
      /** compute h **/
      for( auto i( 0 ); i < m; i++ )
      {
         p = ((p * d) + term[ i ] ) % q;
         if( i < ( m - 1 ) )
         {
            h = ( h * d ) % q;
         }
      }

      input.addPort< char  >( "in"  );
      output.addPort< hit_t   >( "out" );
   }
   
   virtual kstatus run()
   {
      auto &in_port( input[ "in" ] );
      if( in_port.size() < m ) 
      {
         return( raft::stop );
      }
      auto &out_port( output[ "out" ] );
      auto T( in_port.peek_range< char >( in_port.size() ) );
      
      if( ! curr_index_set )
      {
         curr_index = T.getindex(); 
         for( auto i( 0 ); i < m; i++ )
         {
            t = ( ( t * d ) + T[ i ] ) % q; 
         }
         curr_index_set = true;
      }
      
      std::size_t s( 0 );
      do
      {
         if( p == t )
         {
            out_port.push< hit_t >( curr_index );
         }
         const auto remove_val( ( T[ s ]  * h ) % q );
         t = ( t - remove_val ) % q;
         t = ( d * t ) % q;
         t = ( t + T[ s + m  ] ) % q;
         curr_index++;
         s++;
      }while( s <= ( T.size() - m - 1  ) );
      return( raft::stop );
   }

private:
   /** prime number **/
   const std::uint64_t q = 17;
   /** shift **/
   const std::uint64_t d = 0xff;
   /** search term length **/
   std::size_t   m = 0;
   /** term hash pattern **/
   std::uint64_t  p = 0;
   /** max radix power to subtract from rolling hash **/
   std::uint64_t  h = 1;

   /** current index **/
   bool                curr_index_set = false;
   std::size_t         curr_index     = 0;
   /** the current hash of the string m **/
   std::int64_t        t              = 0;
};
#endif /* END _RKSEARCH_TCC_ */
