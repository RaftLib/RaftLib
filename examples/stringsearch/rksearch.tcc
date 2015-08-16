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
   search( const std::string term ) : kernel(),
                                  search_term_length( term.length() ),
                                  pattern( hashfunc( term, search_term_length, d, q ) ),
                                  h( comp_h< std::int64_t >( q, d, search_term_length ) ),
                                  start( term[ 0 ] )
   {
      assert( search_term_length > 0 ); 
      input.addPort< char  >( "in"  );
      output.addPort< hit_t   >( "out" );
   }
   
   virtual kstatus run()
   {
      auto &in_port( input[ "in" ] );
      if( in_port.size() < search_term_length ) 
      {
         return( raft::stop );
      }
      if( ! curr_index_set )
      {
         auto range( in_port.peek_range< char >( search_term_length ) );
         curr_index = range.getindex(); 
         for( std::size_t i( 0 ); i < search_term_length; i++ )
         {
            t = ( ( ( t << 8 ) - t ) + range[ i ] ) % q;
         }
         curr_index_set = true;
      }

      auto &out_port( output[ "out" ] );
      
      auto everything( in_port.peek_range< char >( in_port.size() ) );
      std::size_t s( 0 );
      do
      {
         if( pattern == t )
         {
            out_port.push< hit_t >( curr_index );
         }
         const auto remove_val( ( everything[ s ]  * h ) % q );
         t = ( t + ( ( q << 8 ) - q ) - remove_val ) % q;
         t = ( ( t << 8 ) - t ) % q;
         t = ( t + everything[ s + search_term_length  ] ) % q;
         curr_index++;
         s++;
      }while( s <= (everything.size() - search_term_length - 1) );

      return( raft::stop );
   }

private:
   /** prime number **/
   const std::uint64_t q = 33554393;
   /** shift **/
   const std::uint64_t d = 0xff;
   /** search term length **/
   const std::size_t   search_term_length = 0;
   /** term hash pattern **/
   const std::int64_t  pattern;
   /** max radix power to subtract from rolling hash **/
   const std::int64_t  h = 1;
   /** search term start char **/
   const char          start;

   /** current index **/
   bool                curr_index_set = false;
   std::size_t         curr_index     = 0;
   /** the current hash of the string search_term_length **/
   std::int64_t        t              = 0;
   /** functions **/
   /**
    * hashfunc - rabinkarp rolling hash function, used once for the term
    * and to start off each chunk that the kernel intakes.
    * @param   text - const std::string, text to be hashed
    * @param   length - const std::size_t, length of string assumed to start at zero
    * @return  std::int64_t
    */
   static auto hashfunc( const std::string &text, 
                         const std::size_t length, 
                         const std::uint64_t d,
                         const std::uint64_t q ) -> std::int64_t
   {
      std::int64_t t( 0 );
      for( auto i( 0 ); i < length; i++ )
      {
         t = ( ( ( t << 8 ) - t ) + text[ i ] ) % q;
      }
      return( t );
   }

   template< typename H,
             typename Q, 
             typename D, 
             typename L > static H comp_h( const Q q, 
                                              const D d, 
                                              const L l ) 
   {
      H h2( 1 );
      for( auto i( 1 ); i < l; i++ )
      {
         h2 = ( h2 * d ) % q;
      }
      return( h2 );
   }
};
#endif /* END _RKSEARCH_TCC_ */
