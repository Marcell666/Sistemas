all: programa mainF
programa: programa.c
	gcc -Wall -o $@ $^
mainF: mainF.c fila.c fila.h
	gcc -Wall -o $@ $^
