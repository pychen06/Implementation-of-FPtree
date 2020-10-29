all: main.cpp
	g++ -pthread -std=c++11 -o main main.cpp
clean:
	rm -rf main
