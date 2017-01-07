NonTerminals : {OBJ,A,T,S,U,M}

OBJ :=  raft::kernel
    |   raft::kset

A   :=  T
    |   S
    |   U

T   :=  A '>>' OBJ
    |   OBJ

S   :=  T M '<=' T 

U   :=  T '>=' M T

M   :=  raft::order::out
    |   raft::manip_vec_t
    |   
    ;
