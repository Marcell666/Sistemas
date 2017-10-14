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
#define CHAVE 8180

int contaEspacos(char *nome){
	char *espaco;
	int nEspacos = 0;
	while(TRUE){
		espaco = strchr(nome, ' ');
		if(espaco==NULL) break;
		nEspacos++;
	}
}	

int main(void){
	int i, id, status;
	char *comando;
	char arg[MAX_STRING];
	int segmento;
	
	/* Criando variável compartilhada, vou usá-la para passar os comandos lidos aqui para o escalonador */
	segmento = shmget(CHAVE, MAX_STRING*sizeof(char), IPC_CREAT |  S_IRUSR |S_IWUSR);
	comando = shmat(segmento, NULL, NULL);

	/* O processo escalonador sera filho deste processo */
	id = fork();
	if(id==0)
		execl("./escalonador", NULL);
	
	/* Loop para ler os comandos */
	do{
		printf("Use 'prog a b c..' ou 'exit' para sair.\n");
		scanf(" %80[^\n]", comando);
		printf("comando digitado:\n%s\n", comando);

		if(contaEspacos(comando) > MAX_ARGS)
			printf("maximo de %d argumentos\n", MAX_ARGS);
			continue;
		}
		/* Avisamos ao escalonador que um novo processo precisa ser criado */
		kill(id, SIGUSR1);
		/*
			Paramos este processo, não queremos receber outro antes que o anterior ja tenha sido enviado.
			E não queremos que este processo termine antes do outro.
			Caso este processo termine, o outro deve ser avisado e terminar tambem.
		*/
		kill(getpid(), SIGSTOP);
	}while(strcmp(comando, "exit") != 0);	

	/* Encerrando... */
	shmdt(comando);
	shmctl(segmento);
	return 0;
}
