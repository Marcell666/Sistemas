#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

int main(int argc, char **argv){
	int id, i, e;
	int tempoAtual;
	int rajadas = argc;
	id = getpid();
	printf("%d processo criado\n", getpid());
	for (i=1;i<rajadas;i++){
		tempoAtual = atoi(argv[i]);
		for(e=0;e<tempoAtual;e++){
			printf("%d - %d/%d\n", id,e+1,tempoAtual); //TODO deletar, deixar apenas o de baixo
			//printf("%d\n", id);
			sleep(1);
		}
		printf("%d - termina rajada\n", getpid());
		//kill(getppid(), SIGUSR1);
	}
	printf("processo %d para terminar...\n", id);
	return 0;
}
