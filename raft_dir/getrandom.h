#ifndef __GETRANDOM_H__
#define __GETRANDOM_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void     init();

uint32_t getUniformRandom();

uint32_t getRandom();

uint64_t getUniformRandomInRange(uint64_t range);

#ifdef __cplusplus
}
#endif

#endif /* END __GETRANDOM_H__ */
