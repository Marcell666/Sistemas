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

int main(int argc, char **argv){
	f1 = FILA_cria(2);
	double tempo = 0;


	/* Loop para criar os processos */
	criaNovoProcesso(f1, "programa 2 4 5");
	criaNovoProcesso(f1, "programa 3 1 2");
	criaNovoProcesso(f1, "programa 1 2 1");

	/* Loop para tratar os programas em execução/espera */
	while(TRUE){
		if(tempo-clock()<1000) continue;
		tempo = clock();
		printf("passou 1 u.t. agora estamos em %lf\n", tempo);
		/* Caso um processo tenha extrapolado o tempo, ele deve ser movido para uma fila de nivel mais baixo */
/*
		if(FILA_tempoRestante(fAtual()) < LIM_MIN){
			id = FILA_topId(fAtual());
			FILA_remove(fAtual());
			FILA_insere(fAnt(), id);
		}
*/
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
		Este processo é que chamou o escalonador, então ele fez isso enquanto estava executando e não depois de seu tempo acabar.
		Nos resta saber se ele parou junto com o tempo do escalonador ou antes disso.
	*/
	int id;
	int restante = FILA_atualizaCPU(fAtual(), clock());
	/* dentro da faixa de 'mudar prioridade para cima' */
	if(restante>LIM_MAX){
		id = FILA_topId(fAtual());
		FILA_remove(fAtual());
		FILA_insere(fProx(), id, clock());			
	}
	/* dentro da faixa de 'acabou no tempo certo' */
//	else if(restante>LIM_MIN && restante<=LIM_MAX){
	
//	}
	/*
		dentro da faixa mudar prioridade para baixo
		Noite que isso na verdade não deve acontecer, pois se passar do tempo o proprio escalonador é que deve parar o processo como na funcao acima. Mas isso é so para garantir, assim mesmo que por alguma razão os processos percam a sincronização, isso não terá um grande impacto.	
	*/
	else{
		
	}
}

void processoTermina(int sinal){
	printf("processo %d terminado\n", FILA_topId(fAtual()));
	FILA_remove(fAtual());
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
