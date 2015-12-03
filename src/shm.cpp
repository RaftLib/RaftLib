/**
 * shm.cpp - 
 * @author: Jonathan Beard
 * @version: Thu Aug  1 14:26:34 2013
 */
#include "shm.hpp"
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdlib.h>
#include <errno.h>
#include <cstring>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <cstdint>
#include <cinttypes>
#include <iostream>
#include "getrandom.h"
#include <sstream>

bad_shm_alloc::bad_shm_alloc( const std::string message ) : std::exception(),
                                                            message( message )
{
   /** nothing to do **/
}

const char*
bad_shm_alloc::what() const noexcept 
{
   return( message.c_str() );
}

void
SHM::GenKey( char *buffer, size_t length )
{
   assert( buffer != nullptr );
   const uint32_t key( getUniformRandom() );
   const size_t buffer_size( 1000 );
   char gen_buffer[ buffer_size ];
   memset( gen_buffer, 
           '\0', 
           buffer_size );

   snprintf( gen_buffer, 
             buffer_size, 
             "%" PRIu32 "", 
             key );
   const size_t cp_length( std::min( strlen( gen_buffer ), length ) );
   memset( /* input buffer */ buffer, 
                              '\0', 
                              length );
   strncpy( buffer, 
            gen_buffer,
            cp_length - 1 /* null term */ );
}

void*
SHM::Init( const char *key,
           size_t nbytes,
           bool   zero   /* zero mem */,
           void   *ptr )
{
   assert( key != nullptr );
   const int success( 0 );
   const int failure( -1 );
   int fd( failure  );
   errno = success;
   /* essentially we want failure if the file exists already */
   if( access( key, F_OK ) == success )
   {
      std::stringstream ss;
      ss << "File exists with name \"" << key << "\", error code returned: ";
      ss << strerror( errno );
      throw bad_shm_alloc( ss.str() );
   }
   /* set read/write set create if not exists */
   const int flags( O_RDWR | O_CREAT | O_EXCL );
   /* set read/write by user */
   const mode_t mode( S_IWUSR | S_IRUSR );
   errno = success;
   fd  = shm_open( key, 
                   flags, 
                   mode );
   if( fd == failure )
   {
      std::stringstream ss;
      ss << "Failed to open SHM with file descriptor \"" << key << "\", error code returned: ";
      ss << strerror( errno );
      throw bad_shm_alloc( ss.str() ); 
   }
   /* else begin truncate */
   errno = success;
   if( ftruncate( fd, nbytes ) != success )
   {
      std::stringstream ss;
      ss << "Failed to truncate shm for file descriptor (" << fd << ") ";
      ss << "with number of bytes (" << nbytes << ").  Error code returned: ";
      ss << strerror( errno );
      shm_unlink( key );
      throw bad_shm_alloc( ss.str() );
   }
   /* else begin mmap */
   errno = success;
   void *out( NULL );
   out = mmap( ptr, 
               nbytes, 
               ( PROT_READ | PROT_WRITE ), 
               MAP_SHARED, 
               fd, 
               0 );
   if( out == MAP_FAILED )
   {
      std::stringstream ss;
      ss << "Failed to mmap shm region with the following error: " << strerror( errno ) << ",\n";
      ss << "unlinking.";
      shm_unlink( key );
      throw bad_shm_alloc( ss.str() );
   }
   if( zero )
   {
      /* everything theoretically went well, lets initialize to zero */
      memset( out, 0x0, nbytes );
   }
   return( out );
}

/** 
 * Open - opens the shared memory segment with the file
 * descriptor stored at key.
 * @param   key - const char *
 * @return  void* - start of allocated memory, or NULL if
 *                  error
 */
void*
SHM::Open( const char *key )
{
   assert( key != nullptr );
   /* accept no zero length keys */
   assert( strlen( key ) > 0 );
   const int success( 0 );
   const int failure( -1 );
   int fd( failure );
   struct stat st;
   memset( &st, 
           0x0, 
           sizeof( struct stat ) );
   const int flags( O_RDWR | O_CREAT );
   mode_t mode( 0 );
   errno = success;
   fd = shm_open( key, 
                  flags, 
                  mode ); 
   if( fd == failure )
   {
      std::stringstream ss;
      ss << "Failed to open SHM with key \"" << key << "\", with the following error code: ";
      ss << strerror( errno ); 
      throw bad_shm_alloc( ss.str() );
   }
   /* stat the file to get the size */
   errno = success;
   if( fstat( fd, &st ) != success )
   {
      std::stringstream ss;
      ss << "Failed to stat shm region with the following error: " << strerror( errno ) << ",\n";
      ss << "unlinking.";
      shm_unlink( key );
      throw bad_shm_alloc( ss.str() );
   }
   void *out( NULL );
   errno = success;
   out = mmap( NULL, 
               st.st_size, 
               (PROT_READ | PROT_WRITE), 
               MAP_SHARED, 
               fd, 
               0 );
   if( out == MAP_FAILED )
   {
      std::stringstream ss;
      ss << "Failed to mmap shm region with the following error: " << strerror( errno ) << ",\n";
      ss << "unlinking.";
      shm_unlink( key );
      throw bad_shm_alloc( ss.str() );
   }
   /* close fd */
   close( fd );
   /* done, return mem */
   return( out );
}

bool
SHM::Close( const char *key,
            void *ptr,
            size_t size,
            bool zero,
            bool unlink )
{
   const int success( 0 );
   if( zero )
   {
      memset( ptr, 0x0, size );
   }
   errno = success;
   if( munmap( ptr, size ) != success )
   {
      perror( "Failed to unmap shared memory, attempting to close!!" );
   }
   if( unlink )
   {
      errno = success;
      return( shm_unlink( key ) == success );
   }
   else
   {
      return( true );
   }
}
