all: LargeVis

LargeVis: LargeVis.o main.o
	g++ LargeVis.o main.o -o LargeVis -lm -pthread -lgsl -lgslcblas -Ofast -march=native -ffast-math

LargeVis.o: LargeVis.cpp LargeVis.h ANNOY/*
	g++ LargeVis.cpp -c -Ofast

main.o: main.cpp LargeVis.h ANNOY/*
	g++ main.cpp -c -Ofast

.PHONY: clean
clean:
	rm -f LargeVis *.o