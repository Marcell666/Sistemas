all: programa mainF cmd
programa: programa.c
	gcc -Wall -o $@ $^
mainF: mainF.c fila.c fila.h
	gcc -Wall -o $@ $^
cmd: leCmd.c
	gcc -Wall -o $@ $^
