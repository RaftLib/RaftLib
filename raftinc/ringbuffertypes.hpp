#ifndef RAFTRINGBUFFERTYPES_HPP 
#define RAFTRINGBUFFERTYPES_HPP 1
#include <array>

namespace Type
{
   enum RingBufferType { Heap, 
                         SharedMemory, 
                         TCP, 
                         Infinite, 
                         N };

    static constexpr std::array<  const char[20] , 
                      Type::N > type_prints
                             = {{ "Heap", "SharedMemory", "TCP", "Infinite"  }};
}
   
   enum Direction { Producer, Consumer };
#endif
