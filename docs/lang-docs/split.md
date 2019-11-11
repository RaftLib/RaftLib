## scenarios for split prepend

### **RULE 1** a <= b
1. duplicate 'b' for every _i_ output port in 'a'
2. attach each output port a\_{i} to duplicate kernels b^{i}
3. Done 

### **RULE 2** a <= b <= c
Notes:
* Order of evaluation is left to right so ``a<=b`` is first reduced to ``A'``
* ``A' <= c`` is processed second

Actions:
1. Apply **RULE 1** to ``a<=b`` to reduce to ``A'``, source of ``A'`` is 'a', sink is defined by _i_
clones of 'b' where _i_ is determined by the number of output ports in _a_.
2. For each output port ``A'``, apply **RULE 1** again, resulting in _j_ clones of 'c' for 
every output port in ``A'``

### a <= b >> c

