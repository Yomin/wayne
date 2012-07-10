
all: wayne

wayne: *.hpp *.cpp
	g++ -Wall -o wayne *.cpp
