#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <time.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>

#include "fila.h"

#define MAX_ARGS  10
#define MAX_STRING 80
#define TRUE 1

/*Podiamos criar uma estrutura pra cada processo com seu pid, tempo de i/o e fila que pertence, assim conseguimos colocar ele em uma fila de i/o saber quanto tempo ele tem que ficar lá e a hora de retirar, alem disso podemos guardar o tempo de execução que ele fez (já está na fila) */

ptFila filaIO;
ptFila f1;

ptFila fAtual();
ptFila fReset();
ptFila fProx();
ptFila fAnt();
void criaNovoProcesso(ptFila f, char *comando);
void processoTermina(int sinal);
void processoIO(int sinal);

int tempo;
int solicitouIO;

int main(int argc, char **argv){
	int tempoRestante;
	int id;
	int segmento;
	int* flag;
	int aux;
	char comando[81];
	
	tempo=0;
	solicitouIO=0;

	// aloca a memória compartilhada
	segmento = shmget (8182, sizeof (int), S_IRUSR);	
	// associa a memória compartilhada ao processo
	flag = (int*) shmat (segmento, 0, 0);

	f1 = FILA_cria(2);
	filaIO = FILA_cria(666);

	signal(SIGUSR1, processoIO);
	signal(SIGUSR2, processoTermina);
	
	/* Loop para criar os processos */
	//criaNovoProcesso(f1, "programa 1 3");
	criaNovoProcesso(f1, "programa 2 3");

	//criaNovoProcesso(f1, "programa 2 4 5");
	//criaNovoProcesso(f1, "programa 3 1 2");
	//criaNovoProcesso(f1, "programa 1 2 1");
	id = FILA_topId(fAtual());
	printf("Iniciando processo %d\n", id);
	kill(id, SIGCONT);

	/* Loop para tratar os programas em execução/espera */
	while(TRUE){
		sleep(1);
		tempo++;
		if (*flag==1) //tem coisa pra ler
		{
			aux=read(0, comando,81);
			criaNovoProcesso(f1, comando);
			*flag=0;			
		}
		printf("passou 1 u.t. agora estamos em %d\n", tempo);

		if(!FILA_vazia(filaIO))
			FILA_atualizaIO(filaIO, tempo);
		if(FILA_vazia(fAtual()))
			continue;


		id = FILA_topId(fAtual());
		tempoRestante = FILA_tempoRestante(fAtual(), tempo);
		printf("tempoRestante:%d do processo %d\n", tempoRestante, id);
	
		/* Nem o tempo acabou, nem o programa solicitou I/O, então basta continuar executando */
		if(!solicitouIO && tempoRestante>=0)
			continue;


		if(id>0)
			kill(id, SIGSTOP);
		else{
			printf("BUG NOUTRO LUGAR\n");
		}

		/* Caso um processo tenha extrapolado o tempo, ele deve ser movido para uma fila de nivel mais alto e menor prioridade */
		if(!solicitouIO && tempoRestante<0 ){
			printf("colocando processo %d numa fila mais alta\n", id);	
			FILA_remove(fAtual());
			FILA_insere(fProx(), id, tempo);//coloca numa mais alto	
		}
		/* Caso um processo tenha terminado antes do tempo esperado, ele deve ser movido para uma fila de nivel mais baixo e maior prioridade.
		Temos que colocar o processo que pediu I/O em uma fila de I/O */
		else if(solicitouIO && tempoRestante >=0){
			//FILA_insere(fAnt(), id, tempo);//coloca numa mais baixo	
			printf("colocando processo %d numa fila mais baixa\n", id);	
			printf("colocando processo %d em I/O\n", id);
			FILA_comecaIO(fAtual(), fAnt(), filaIO, tempo);		
		}
		/*	Dentro da faixa de 'acabou no tempo certo'
			Caso o processo tenha terminado no tempo correto, ele é movido para o final da fila 
		Temos que colocar o processo que pediu I/O em uma fila de I/O */	
		else if(solicitouIO && tempoRestante<0){
			//FILA_insere(fAtual(), id, tempo);//coloca na mesma
			printf("colocando processo %d no final da mesma fila\n", id);
			printf("colocando processo %d em I/O\n", id);			
			FILA_comecaIO(fAtual(), fAtual(), filaIO, tempo);
			
		}

		id = FILA_topId(fAtual());
		solicitouIO = 0;

		if(id==-1){
			printf("fila vazia\n");
			if(FILA_vazia(filaIO)){
				printf("Nenhum processo restante, fechando o programa");
			
				/* Encerrando... */
				FILA_libera(f1);
				FILA_libera(filaIO);
				exit(0);
			}
		}
		else{
			printf("executando processo %d, que e o proximo da fila\n", id);
			FILA_comecaCPU(fAtual(), tempo);

			if(id>0)
				kill(id, SIGCONT);
			else{
				printf("BUG ACOLA\n");
			}
		}
	}

	/* Encerrando... */
	// libera a memória compartilhada do processo
	shmdt(flag);
	//libera Fila
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
		printf("criado:%d por: %d - tempo %d - Novo processo\n%s\n", getpid(), getppid(), tempo, comando);
		execv("./programa", args);
		exit(1);
	}
	FILA_insere(f,id, 0);
	printf("getpid %d parando processo %d\n", getpid(), id);
	if(id>0)
		kill(id, SIGSTOP);
	else{
		printf("BUG AQUI\n");
	}
	
}

void processoIO(int sinal){
	printf("Processo %d solicitou IO em tempo %d\n", FILA_topId(fAtual()), tempo);
	solicitouIO = 1;
}

void processoTermina(int sinal){
	int id;
	printf("processo %d terminado\n", FILA_topId(fAtual()));
	id = FILA_topId(fAtual());
	FILA_remove(fAtual());
		
	//TODO colocar f2 e f3 tambem
	if(FILA_vazia(filaIO) && FILA_vazia(f1)){
		printf("todos os processos encerraram, terminando programa\n");
		
		/* Encerrando... */
		FILA_libera(f1);
		FILA_libera(filaIO);
		exit(0);
	}
	if(id>0)
		kill(id, SIGCONT);
	else{
		printf("BUG ALI\n");
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
