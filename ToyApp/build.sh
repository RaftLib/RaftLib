rm -rf main.o sumapp; g++ -std=c++11 -O3 -g -I../ -I ../fifo -c main.cpp -DRDTSCP=1; g++ -std=c++11 -O3 -g -o sumapp main.o raftlight.a -lrt 
