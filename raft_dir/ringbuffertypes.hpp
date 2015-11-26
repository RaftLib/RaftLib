#ifndef __RINGBUFFERTYPES__ 
#define __RINGBUFFERTYPES__ 1
namespace Type{
   enum RingBufferType { Heap, SharedMemory, TCP, Infinite, N};
}
   
   enum Direction { Producer, Consumer };
#endif
