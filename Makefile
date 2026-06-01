all: main.cpp
	clang++ main.cpp -lsndio -o jrecord
