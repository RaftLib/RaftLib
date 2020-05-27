#ifndef RAFTRINGBUFFERTYPES_HPP 
#define RAFTRINGBUFFERTYPES_HPP 1

namespace Type{
   enum RingBufferType { Heap, 
                         SharedMemory, 
                         TCP, 
                         Infinite, 
                         N };
}
   
   enum Direction { Producer, Consumer };
#endif
