#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <time.h>

#include "fila.h"

#define MAX_ARGS  10
#define MAX_STRING 80
#define TRUE 1
#define LIM_MAX 0.5D
#define LIM_MIN -0.5D

ptFila f1;

ptFila fAtual();
ptFila fReset();
ptFila fProx();
ptFila fAnt();
void criaNovoProcesso(ptFila f, char *comando);
void processoTermina(int sinal);
void processoIO(int sinal);

int solicitouIO;

int main(int argc, char **argv){
	int tempo=0;
	int tempoRestante;
	int id;
	f1 = FILA_cria(2);
	solicitouIO=0;


	signal(SIGUSR1, processoIO);
	signal(SIGUSR2, processoTermina);
	

	/* Loop para criar os processos */
	criaNovoProcesso(f1, "programa 2 4 5");
	criaNovoProcesso(f1, "programa 3 1 2");
	criaNovoProcesso(f1, "programa 1 2 1");
	id = FILA_topId(fAtual());
	kill(id, SIGCONT);

	/* Loop para tratar os programas em execução/espera */
	while(TRUE){
		sleep(1);
		printf("passou 1/4 de u.t. agora estamos em %lf\n", tempo);

		tempoRestante = FILA_tempoRestante(fAtual(), tempo);
		printf("tempoR1:%lf\n", tempoRestante);

		if(!solicitou && tempoRestante>0) continue;


			id = FILA_topId(fAtual());
			kill(id, SIGSTOP);
			printf("colocando processo %d numa fila mais baixa\n", id);
			FILA_remove(fAtual());

			/* Caso um processo tenha extrapolado o tempo, ele deve ser movido para uma fila de nivel mais baixo */
			if(!solicitouIO && tempoRestante<=0 )
				FILA_insere(fAnt(), id, tempo);//coloca numa mais baixa
			else if(solicitouIO && tempoRestante >0)
				FILA_insere(fProx(), id, tempo);//coloca numa mais alta
			/*	Dentro da faixa de 'acabou no tempo certo'
				Caso o processo tenha terminado no tempo correto, ele é movido para o final da fila */
			else if(solicitouIO && tempoRestante<=0)
				FILA_insere(fAtual(), id, tempo);//coloca na mesma			

			id = FILA_topId(fAtual());
			printf("executando processo %d, que e o proximo da fila\n", id);
			kill(id, SIGCONT);
			solicitouIO = 0;

		
		tempo++;
	}

	/* Encerrando... */
	FILA_libera(f1);
	return 0;
}

/* Funcoes de controle de processos */

void criaNovoProcesso(ptFila f, char *comando){
	int id=0;
	int i=1;
	char arg[MAX_STRING];
	char *args[MAX_ARGS];
	strcpy(arg, comando);
	args[0] = strtok(arg, " ");
	while(TRUE){
		args[i] = strtok(NULL, " ");
		if(args[i]==NULL) break;
		i++;
	}
	id = fork();
	if(id==0){
		printf("%d - Novo processo\n%s\n", getpid(), comando);
		raise(SIGSTOP);
		execv("./programa", args);
	}
	FILA_insere(f,id, tempo );
	
}

void processoIO(int sinal){
	solicitouIO = 1;
}

void processoTermina(int sinal){
	printf("processo %d terminado\n", FILA_topId(fAtual()));
	FILA_remove(fAtual());
	//TODO colocar f2 e f3 tambem
	if(FILA_vazia(f1)){
		printf("todos os processos encerraram, terminando programa\n");
		
		/* Encerrando... */
		FILA_libera(f1);
		exit(1);
	}
}

/* Funcoes de controle de fila */

ptFila fAtual(){
//	if(indexAtual<0 ||indexAtual>2) return NULL;
//	return filas[indexAtual];
	return f1;
}

/* 
	Retorna primeira fila nao vazia, atribui index a indexAtual.
	Retorna NULL se nenhuma fila estiver vazia, atribui -1 a indexAtual.
*/
ptFila fReset(){
//	int i;
//	indexAtual = 0;
//	for(i=0;i<NFILAS;i++){
//		if(!FILA_vazia(filas[i])){
//			indexAtual = i;
//			return filas[indexAtual];
//		}
//	}
//	indexAtual = -1;
	return NULL;
}

/* Retorna fila, um nivel de prioridade maior do que a atual */
ptFila fProx(){
//	int i =  (indexAtual+1)%3;
//	return filas[i];
	return f1;
}

/* Retorna fila, um nivel de prioridade menor do que a atual */
ptFila fAnt(){
//	int i = indexAtual-1;
//	if (i<0)i = NFILAS-1;
//	return filas[i];
	return f1;
}