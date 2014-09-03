#include <iostream>
#include <cstdlib>
#include <cstdint>
#include <thread>
#include <functional>
#include <cassert>
#include <vector>

class FooBase 
{
public:
   FooBase()
   {
   }

   template < class T > void insert( T &input )
   {
      insert( ((void*)&input) );
   }


   template < class T > T get( std::size_t n )
   {
      void *ptr;;
      remove( &ptr, n );
      T *val( reinterpret_cast< T* >( ptr ) );
      return( *val );
   }

   virtual void print() = 0;

protected:
   virtual void insert( void *item ) = 0;
   virtual void remove( void **item, std::size_t n ) = 0;
};
   
template < class T > class FooA : public FooBase
{
public:
   FooA() : FooBase()
   {
   }

   virtual void print()
   {
      std::cout << "Printing from FooA: \n";
      for( T &item : list )
      {
         std::cout << item << "\n";
      }
   }
protected:
   virtual void insert( void *item )
   {
      T *cast_item = reinterpret_cast< T* >( item );
      list.push_back( *cast_item );
   }

   virtual void remove( void **item, std::size_t n )
   {
      assert( list.size() >= n );
      *item = (void*) &(list[ n ]);
   }
private:
   std::vector< T > list;
};

template< class T > class FooB : public FooBase
{
public:
   FooB() : FooBase()
   {
   }

   virtual void print()
   {
      std::cout << "Printing from FooB: \n";
      for( T &item : list )
      {
         std::cout << item << "\n";
      }
   }
protected:
   virtual void insert( void *item )
   {
      T *cast_item = reinterpret_cast< T* >( item );
      list.push_back( *cast_item );
   }
   
   virtual void remove( void **item, std::size_t n )
   {
      assert( list.size() >= n );
      *item = (void*) &(list[ n ]);
   }
private:
   std::vector< T > list;
};

int
main( int argc, char **argv )
{
   std::vector< FooBase* > list;
   FooBase *a( new FooA< int >() );
   FooBase *b( new FooB< float >() );

   list.push_back( a );
   list.push_back( b );
   
   int var( 0 );
   for( ; var< 10; var++ )
   {
      a->insert( var );
      var++;
   }
   for( ; var < 20; var++ )
   {
      float t( (float) var );
      b->insert( t );
      var++;
   }
   
  
   for( FooBase *ptr : list )
   {
      ptr->print();
   }

   std::cout << "Observing: " << a->get< int  >( 1 ) << "\n";
   std::cout << "Observing: " << b->get< float >( 2 ) << "\n"; 

   return( EXIT_SUCCESS );
}
