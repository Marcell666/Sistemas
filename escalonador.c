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

static char *comando;
Fila f1,f2,f3, fAtual;

int main(int argc, char **argv){
	Fila filas[3] = {f1, f2, f3};
	int i, id, status;
	int idProcRodando;
	int segmento;
	int tempo=0;
	FILA_cria(f1, 1);
	FILA_cria(f2, 2);
	FILA_cria(f3, 4);
	fAtual = f1;
	iAtual = 0;

	/* Criando variável compartilhada, vou usá-la para passar os comandos lidos aqui para o escalonador */
	segmento = shmget(CHAVE, MAX_STRING*sizeof(char), IPC_EXCL | S_IRUSR |S_IWUSR);
	comando = shmat(segmento, NULL, NULL);

	/* Usando sinais para cuidar dos programas novos */
	signal(SIGUSR1, criaNovoProcesso());

	/* Loop para tratar os programas em execução/espera */
	do{
		printf("passou 1 u.t. agora estamos em %d\n", tempo);
	
		if(FILA_topTempo(fAtual) == 0){
			id = FILA_topTempo(fAtual);
			FILA_remove(fAtual);
			FILA_insere(filas[iAtual+1], id);
		}

		FILA_atualiza(fAtual);
		sleep(1);
	}while(strcmp(comando, "exit") != 0);	

	/* Encerrando... */
	shmdt(comando);
	shmctl(segmento);
	FILA_libera(f1);
	FILA_libera(f2);
	FILA_libera(f3);
	return 0;
}

void criaNovoProcesso(int sinal){
	int id=0;
	int i=0;
	char args[MAX_ARGS][MAX_STRING];
	printf("Novo processo a ser criado %d\n", sinal);
	while(TRUE){
		args[i] = strtok(comando, " ");
		if(args[i]==NULL) break;
		i++;
	}
	id = fork();
	if(id==0)
		execv("./programa", args);
	FILA_insere(f1,id);
	kill(getppid(), SIGCONT);
}
