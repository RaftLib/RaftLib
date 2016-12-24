For main readme, see root directory, these are notes
on this directory only.

NOTES:
1) Enums in defs.hpp, specifically the raft::vm::type seem to 
have issues with g++ 5.4, throwing an error when used in combination
with the variadic template in kernel.hpp. Solution is simply to 
upgrade to a newer version of gcc, confirmed it works and compiles
on gcc 6.3+.


