#ifndef _AFFINITY_HPP_
#define _AFFINITY_HPP_  1

#include <cstddef>

struct affinity
{
   static void set( const std::size_t desired_core );
};
#endif /* END _AFFINITY_HPP_ */
