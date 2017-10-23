#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
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

int main(int argc, char **argv){
	f1 = FILA_cria(2);
	double tempo = 0;
	double tempoRestante;
	int id;

	signal(SIGUSR1, processoIO);
	signal(SIGCHLD, processoTermina);

	/* Loop para criar os processos */
	criaNovoProcesso(f1, "programa 2 4 5");
	criaNovoProcesso(f1, "programa 3 1 2");
	criaNovoProcesso(f1, "programa 1 2 1");

	/* Loop para tratar os programas em execução/espera */
	while(TRUE){
		if(tempo-clock()<250) continue;
		tempo = clock();
		printf("passou 1/4 de u.t. agora estamos em %lf\n", tempo);

		tempoRestante = FILA_tempoRestante(fAtual(), clock());
		/* Caso um processo tenha extrapolado o tempo, ele deve ser movido para uma fila de nivel mais baixo */
		printf("tempoR1:%lf\n", tempoRestante);
		if(tempoRestante < LIM_MIN){
			id = FILA_topId(fAtual());
			printf("colocando processo %d numa fila mais baixa\n", id);
			FILA_remove(fAtual());
			FILA_insere(fAnt(), id, clock());
		}
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
	printf("Novo processo a ser criado\n%s\n", comando);
	strcpy(arg, comando);
	args[0] = strtok(arg, " ");
	while(TRUE){
		args[i] = strtok(NULL, " ");
		if(args[i]==NULL) break;
		i++;
	}
	printf("%s\n", comando);
	id = fork();
	if(id==0)
		execv("./programa", args);
	FILA_insere(f,id, clock() );
}

void processoIO(int sinal){
	/*
		Este processo é aquele que chamou o escalonador, então ele fez isso enquanto estava executando e não depois de seu tempo acabar.
		Nos resta saber se ele parou junto com o tempo do escalonador ou antes disso.
	*/
	int id;
	double tempoRestante = FILA_tempoRestante(fAtual(), clock());
	printf("tempoR2:%lf\n", tempoRestante);
	id = FILA_topId(fAtual());
	FILA_remove(fAtual());
	/*	Dentro da faixa de 'mudar prioridade para cima'
		Caso um processo tenha terminado antes do tempo acabar, ele deve ser movido para uma fila de nivel mais alto */
	if(tempoRestante>LIM_MAX){
		printf("colocando processo %d numa fila mais alta\n", id);
		FILA_insere(fProx(), id, clock());
	}
	/*	Dentro da faixa de 'acabou no tempo certo'
		Caso o processo tenha terminado no tempo correto, ele é movido para o final da fila */
	else{
		printf("colocando processo %d na mesma fila\n", id);
		FILA_insere(fAtual(), id, clock());
	}
}

void processoTermina(int sinal){
	printf("processo %d terminado\n", FILA_topId(fAtual()));
	FILA_remove(fAtual());
	//TODO colocar f2 e f3 tambem
	if(FILA_vazia(f1)){
		printf("todos os procesos encerraram, terminando programa\n");
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
