all:	pc441 doc

pc441:	simulator.c
	gcc -o simulator simulator.c -lm

doc:
	doxygen

clean:
	rm -rf simulator html man latex
