all :
	g++ -Wall -O2 -o calc expr.cc

clean :
	rm calc *.o || :
