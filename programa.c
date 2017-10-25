#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>

int main(int argc, char **argv){
	int id, i, e;
	int rajadaAtual;
	int rajadas = argc - 1;
	id = getpid();
	printf("%d processo criado \n", getpid() );
	//raise(SIGSTOP);
	for (i=0;i<rajadas;i++){
		rajadaAtual = atoi(argv[i+1]);
		for(e=0;e<rajadaAtual;e++){
			printf("%d processo - %d/%d\n", id,e,rajadaAtual); //TODO deletar, deixar apenas o de baixo
			//printf("%d\n", id);
			sleep(1);
		}
		printf("%d - termina rajada %d/%d\n", id, e, rajadaAtual);
		kill(getppid(), SIGUSR1);
	}
	kill(getppid(), SIGUSR2);
	printf("processo %d para terminar...\n", id);
	return 0;
}
