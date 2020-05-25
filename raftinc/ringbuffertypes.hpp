#ifndef RINGBUFFERTYPES 
#define RINGBUFFERTYPES 1
namespace Type{
   enum RingBufferType { Heap, SharedMemory, TCP, Infinite, N_TYPE};
}
   
   enum Direction { Producer, Consumer };
#endif
