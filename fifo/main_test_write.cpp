#include <cstdlib>
#include <iostream>
#include <chrono>
#include <thread>
#include <cstdint>
#include <unistd.h>
#include "ringbuffer.tcc"
#include "SystemClock.tcc"
#include <array>
#include <sstream>
#include <fstream>
#include "randomstring.tcc"
#include "signalvars.hpp"

struct Data
{
   Data( size_t send ) : send_count(  send )
   {}
   std::int64_t          send_count;
} data( 1e2 );

#define SIZE 65536

struct TestData
{
	TestData()
	{
		std::memset( all, 'a', SIZE - 1 );
		all[ SIZE - 1 ] = '\0';
	}

	TestData( const TestData &other )
	{
		for( int i = 0; i < SIZE; i++ )
		{
			all[ i ] = other.all[ i ];
		}
	}

	void operator = ( const TestData &other )
	{
		for( int i = 0; i < SIZE; i++ )
		{
			all[ i ] = other.all[ i ];
		}
	}
	
	bool operator == ( const TestData &other )
	{
		for( int i = 0; i < SIZE; i++ )
		{
			if( all[i] != other.all[i] )
			{
				return( false );
			}
		}
		return( true );
	}
		char all[ SIZE ];
} __attribute__(( aligned( 32 )));
//#define USESharedMemory 1
#define USELOCAL 1
#define BUFFSIZE 100

#ifdef USESharedMemory
typedef RingBuffer< std::int64_t, RingBufferType::SharedMemory, BUFFSIZE > TheBuffer;
#elif defined USELOCAL
typedef RingBuffer< TestData >  TheBuffer;
#endif


Clock *system_clock = new SystemClock< Cycle >( 1 );


void
producer( Data &data, TheBuffer &buffer )
{
   std::int64_t current_count( 0 );
   TestData d;
   while( current_count++ < data.send_count )
   {
      buffer.push( d , 
         (current_count == data.send_count ? 
          RBSignal::RBEOF : RBSignal::RBNONE ) );
   }
   return;
}
void 
consumer( Data &data , TheBuffer &buffer )
{
   TestData test_object;
   while( buffer.get_signal() != RBSignal::RBEOF )
   {
      auto current_count = buffer.pop();
      if( ! (test_object == current_count) )
      {
      	std::cerr << "Not equal, current value is: \n";
      	std::cerr << current_count.all << "\n";
      }
   }
   return;
}

void test()
{
#ifdef USESharedMemory
   char shmkey[ 256 ];
   SharedMemory::GenKey( shmkey, 256 );
   std::string key( shmkey );
   
   RingBuffer< std::int64_t, 
               RingBufferType::SharedMemory, 
               BUFFSIZE > buffer_a( key, 
                                    Direction::Producer, 
                                    false);
   RingBuffer< std::int64_t, 
              RingBufferType::SharedMemory, 
              BUFFSIZE > buffer_b( key, 
                                   Direction::Consumer, 
                                   false);

   std::thread a( producer, 
                  std::ref( data ), 
                  std::ref( buffer_a ) );

   std::thread b( consumer, 
                  std::ref( data ), 
                  std::ref( buffer_b ) );

   
#elif defined USELOCAL
   TheBuffer buffer( BUFFSIZE );
   std::thread a( producer, 
                  std::ref( data ), 
                  std::ref( buffer ) );

   std::thread b( consumer, 
                  std::ref( data ), 
                  std::ref( buffer ) );
#endif
   a.join();
   b.join();

}


int 
main( int argc, char **argv )
{
   //RandomString< 50 > rs;
   //const std::string root( "/project/mercury/svardata/" );
   //const std::string root( "" );
   //std::ofstream ofs( root + rs.get() + ".csv" );
   //if( ! ofs.is_open() )
   //{
   //   std::cerr << "Couldn't open ofstream!!\n";
   //   exit( EXIT_FAILURE );
   //}
   int runs( 1 );
   while( runs-- )
   {
       test();
   }
   //ofs.close();
   if( system_clock != nullptr ) 
      delete( system_clock );
}

