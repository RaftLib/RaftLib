#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#include "getrandom.h"

#ifndef FAIL 
#define FAIL -1
#endif

uint32_t getUniformRandom()
{
#ifdef DEBUG
   return( 1 );
#else
   FILE *fp = fopen("/dev/urandom","r");
   if(fp == NULL)
   {
      const char *err = "Error, couldn't open /dev/urandom!!\n";
      perror(err);
      exit(FAIL);
   }
   uint32_t uniformRandomOutput;
   const size_t numberRead = fread(&uniformRandomOutput, sizeof(uint32_t), 1, fp);
   if(numberRead != 1)
   {
      const char *err = "Error, incorrect number of integers retured!!\n";
      perror(err);
      exit(FAIL);
   }
   else
   {
      fclose(fp);
   }
   return (uniformRandomOutput);
#endif
}


uint32_t getRandom()
{
#ifdef DEBUG
   return( 1 );
#else
   FILE *fp = fopen("/dev/random","r");
   if(fp == NULL)
   {
      const char *err = "Error, couldn't open /dev/random!!\n";
      perror(err);
      exit(FAIL);
   }
   uint32_t randomOutput;
   const size_t numberRead = fread(&randomOutput, sizeof(uint32_t), 1, fp);
   if(numberRead != 1)
   {
      const char *err = "Error, incorrect number of integers retured!!\n";
      perror(err);
      exit(FAIL);
   }
   else
   {
      fclose(fp);
   }
   return (randomOutput);
#endif
}
