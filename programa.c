#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv){
	int id, i, e;
	int tempoAtual;
	int rajadas = argc;
	id = getpid();
	for (i=0;i<rajadas;i++){
		tempoAtual = atoi(argv[i]);
		for(e=0;e<tempoAtual;e++){
			printf("%d\n", id);
			sleep(1);
		}
		kill(getppid(), SIGUSR2);
	}
	printf("processo %d para terminar...\n", id);
	return 0;
}
