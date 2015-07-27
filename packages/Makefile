compile: main.cpp 
	$(CXX) -std=c++11 -O0 -g -c -o main.o main.cpp
	$(CXX) -std=c++11 -O0 -g -o test main.o -lraft

.PHONY: clean
clean:
	rm -rf main.o test
