RINGBUFFERDIR = ./fifo/ 

RBCFILES   = getrandom 
RBCXXFILES = pointer shm Clock procwait resolution fifo signal clockimpl


RBCOBJS		= $(addprefix ./fifo/, $(RBCFILES))
RBCXXOBJS	= $(addprefix ./fifo/, $(RBCXXFILES))
