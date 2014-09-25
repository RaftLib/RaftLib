rm -rf main.o sumapp; clang++ -std=c++11 -O0 -g -I../ -I ../fifo -c main.cpp -DRDTSCP=1; clang++ -std=c++11 -O0 -g -o sumapp main.o raftlight.a 
