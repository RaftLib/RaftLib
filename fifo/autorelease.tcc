/**
 * autorelease.tcc - objects that are created with the express 
 * purpose of handling the release and pushing of data to the
 * consumer kernel without the user having to take any action.
 * Each object is designed to handle pointers or references
 * instead of actual data by copy so that the overhead is 
 * minimal and actual actions are handled by the constructor
 * or destructor.
 * @author: Jonathan Beard
 * @version: Fri Jan  2 19:29:36 2015
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
#ifndef _AUTORELEASE_TCC_
#define _AUTORELEASE_TCC_  1

template< class T > using pop_range_t = std::vector< std::pair< T , raft::signal > >;

enum autotype { allocatetype, 
                poptype, 
                peekrange,
                allocaterange };

template< class T, autotype type = poptype > class autorelease
{
public:
   /**
    * constructor,only used by the fifo
    * @param fifo - FIFO&, current fifo that the object points to
    */
   autorelease( FIFO &fifo ) : fifo( fifo ),
                               item( fifo.peek< T >( &signal ) )
   {}
   /** copy constructor **/
   autorelease( const autorelease< T, type >  &other ) : 
      fifo( other.fifo ),
      item( other.item ),
      signal( other.signal )
   {
      /**
       * NOTE: I hate doing this here, but can't quite get 
       * move semantics to work the way I'd like so this works,
       * and there's not much more than references to copy 
       * anyways if the compiler can't figure out how to inline
       * the object return for the rvalue which we're assigning
       * to the lvalue with an auto type
       */
      const_cast< autorelease< T, type >& >( other ).copied = true;
   }
   
   /**
    * destructor - handles poping of the object
    * which is implemented as a peek, requiring the
    * call of unpeek() and recycle(1).
    */
   ~autorelease()
   {
      if( ! copied )
      {
         fifo.unpeek();
         fifo.recycle( 1 );
      }
   }

   /**
    * *operator - dereference this auto-release objec to 
    * get the actual value (by reference).
    * @return T&
    */
   T& operator *()
   {
      return( item );
   }
   /**
    * sig - get the current signal
    * @return raft::signal
    */
   raft::signal& sig()
   {
      return( (this)->signal );
   }
private:
   /** reference to fifo, needed for peek ops **/
   FIFO         &fifo;
   /** copy of signal at the current peek point **/
   raft::signal signal;
   /** reference to item, retrieved via peek call **/
   T            &item;
   /** necessary in case the compiler doesn't optimize copy **/
   bool         copied = false;
};

template < class T > class autorelease< T, peekrange >
{
public:
   autorelease( FIFO             &fifo, 
                   T * const      queue,
                Buffer::Signal   &sig,
                const std::size_t curr_read_ptr,
                const std::size_t n_items ) : fifo( fifo ),
                                              queue( queue ),
                                              signal( sig ),
                                              crp  ( curr_read_ptr ),
                                              n_items( n_items ),
                                              queue_size( fifo.capacity() )
   {
      
   }

   autorelease( const autorelease< T, peekrange > &other ) : 
      fifo( other.fifo ),
      queue( other.queue ),
      signal( other.signal ),
      crp( other.crp ),
      n_items( other.n_items ),
      queue_size( other.queue_size )
   {
      const_cast< autorelease< T, peekrange >& >( other ).copied = true;
   }
    
   ~autorelease()
   {
      if( ! copied )
      {
         fifo.unpeek();
      }
   }

   T& operator []( const std::size_t index )
   {
      if( index >= n_items )
      {
         std::stringstream ss;
         ss << "Index (" << index << ") out of bounds, "
         << "max value is (" << (n_items - 1) << ")\n";
         throw std::length_error( ss.str() );
      }
      else
      {
         std::size_t ptr_val( (index + crp) % queue_size );
         return( queue[ ptr_val ] );
      }
   }

   raft::signal& sig()
   {
      return( signal.sig() );
   }

   std::size_t getindex()
   {
      return( signal.getindex() );
   }

   std::size_t size() const
   {
      return( n_items );
   }

private:
   FIFO             &fifo;
   T  *  const       queue;
   Buffer::Signal   &signal;
   const std::size_t crp;
   const std::size_t n_items;
   const std::size_t queue_size;
   bool              copied = false;
};


template < class T > class autorelease< T, allocatetype >
{
public:
   
   autorelease(  T *item, FIFO &fifo ) : item( item ), fifo( fifo )
   {
   }

   autorelease( const autorelease< T, allocatetype > &other ) : 
      item( other.item ), 
      fifo( other.fifo )
   {
      const_cast< autorelease< T, allocatetype >& >( other ).copied = true;
   }
   
   ~autorelease()
   {
      if( ! copied )
      {
         fifo.send( signal );   
      }
   }

   T& operator *()
   {
      return( (*item) ); 
   }

   raft::signal& sig()
   {
      return( (this)->signal );
   }

private:
   T                   *item;
   raft::signal         signal = raft::none;
   FIFO                 &fifo;
   bool                 copied = false;
};
#endif /* END _AUTORELEASE_TCC_ */
