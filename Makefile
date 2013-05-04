all :
	g++ -Wall -O2 -o calc queens.cc global.cc expr.cc eval.cc memory.cc

clean :
	rm calc *.o || :
