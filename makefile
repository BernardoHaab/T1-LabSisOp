
all: combile run

combile:
	g++ ./src/server.cpp -o server

run:
	./server
