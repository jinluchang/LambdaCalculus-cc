all :
	g++ -Wall -O2 -o calc expr.cc queens.cc

clean :
	rm calc *.o || :
