
benchmark: benchmark.c args.c args.h Makefile
	g++ -g -o4  -std=c++0x  -o benchmark benchmark.c args.c -lpthread

java:
	java -jar ~/nb/benchmarkJava/dist/benchmarkJava.jar  tree

c:
	~/nb/benchmarkJava/src/benchmarkJava/benchmark tree

