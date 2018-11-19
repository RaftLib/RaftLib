## Notes
Reworked the original a bit. This should be more flexible 
and also allow generation of strings. 


## Grammar 
```c
S   := A
    |
    ;

MODIFIER   :=  raft::order::out
           |   raft::manip_vec_t
           ;

SIMPLECONNECTOR     := '>>'
                    |  '<='
                    |  '>='
                    ;

JOIN    := SIMPLECONNECTOR
        |  SIMPLECONNECTOR B SIMPLECONNECTOR
        ;


OBJ :=  raft::kernel
    |   raft::kset
    ;

A   := OBJ JOIN A
    |  OBJ
    ;

B   :=  MODIFIER '>>' B
    |   MODIFIER
    ;
```
