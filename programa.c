#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv){
	int i, e;
	int tempoAtual;
	int rajadas = argc;
	
	for (i=0;i<rajadas;i++){
		tempoAtual = atoi(argv[i]);
		for(e=0;e<tempoAtual;e++){
			printf("%d\n", getpid());
			sleep(1);
		}
		kill(getppid(), SIGUSR2);
	}
	return 0;
}
