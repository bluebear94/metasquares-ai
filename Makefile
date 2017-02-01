CXX=g++
CXX_FLAGS=-std=c++14 -Wall -Wextra -Werror -pedantic -O3 -g

all: Game.o
	$(CXX) $(CXX_FLAGS) -o msqai Game.o

Game.o: Game.h Game.cpp
	$(CXX) $(CXX_FLAGS) -o Game.o -c Game.cpp