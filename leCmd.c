#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

//só pode ter 10 argumentos
#define MAX_ARGS 10
#define MAX_STRING 80
#define TRUE 1

int contaEspacos(char *nome){
	char *espaco;
	int nEspacos = 0;
	while(TRUE){
		espaco = strchr(nome, ' ');
		if(espaco==NULL) break;
		nEspacos++;
	}
	return nEspacos;
}	

int main(void){
	int i, id;
	char comando[MAX_STRING];
	int fd[2];

	if(pipe(fd)){
		printf("erro! ao criar pipe\n");
		exit(1);
	}
	

	printf("Digite a quantidade de programas que devem ser criados no início.\n");
	scanf("%d", &i);

	/* O processo escalonador sera filho deste processo */
	id = fork();
	if(id==0){
		dup2(fd[0], 0);
		dup2(fd[1], 1);
		execl("./escalonador", "/escalonador", NULL);
	}
	
	close(fd[0]);

	write(fd[1], "%d", i); 

	/* Loop para ler os comandos */
	for (;i>0;i--){
		printf("Use 'prog a b c..'.\n");
		scanf(" %80[^\n]", comando);
		printf("%d - comando digitado:\n%s\n", getpid(), comando);

		if(contaEspacos(comando) > MAX_ARGS){
			printf("maximo de %d argumentos\n", MAX_ARGS);
			continue;
		}

		write(fd[1], comando, strlen(comando)+1);
	}

	return 0;
}
