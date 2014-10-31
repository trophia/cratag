all: cratag.exe

cratag.exe: main.cpp
	g++ --std=c++11 -o$@ $<
