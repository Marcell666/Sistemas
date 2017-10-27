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
#define NFILAS 3
#define CHAVE 8180

/*Podiamos criar uma estrutura pra cada processo com seu pid, tempo de i/o e fila que pertence, assim conseguimos colocar ele em uma fila de i/o saber quanto tempo ele tem que ficar lá e a hora de retirar, alem disso podemos guardar o tempo de execução que ele fez (já está na fila) */

ptFila filaIO;
ptFila filaAtual;
ptFila f1,f2,f3;
ptFila filas[3];


ptFila fAtual();
ptFila fReset();
ptFila fProx();
ptFila fAnt();
void criaNovoProcesso(ptFila f, char *comando);
void processoTermina(int sinal);
void processoIO(int sinal);

void encerra(int status);

int tempo;
int solicitouIO;
int solicitouTermino;
int indexAtual;
int* flag;
int segmento;

int main(int argc, char **argv){
	int tempoRestante;
	int id;
	char comando[81];

	/*funcao cria*/
	//int id=0;
	int i=1;
	char arg[MAX_STRING];
	char *args[MAX_ARGS];
	
	f1 = FILA_cria(1, 0);
	f2 = FILA_cria(2, 1);
	f3 = FILA_cria(4, 2);
	filaIO = FILA_cria(666, 3);
	
	filaAtual = f1;
	
	filas[0] = f1;
	filas[1] = f2;
	filas[2] = f3;
	
	tempo=0;
	solicitouIO=0;
	indexAtual = 0;

	// aloca a memória compartilhada
	segmento = shmget (CHAVE, sizeof (int), S_IRUSR);	
	// associa a memória compartilhada ao processo
	flag = (int*) shmat (segmento, 0, 0);

	signal(SIGUSR1, processoIO);
	signal(SIGUSR2, processoTermina);
	
	/* Loop para criar os processos */

	while (*flag>0) //tem coisa pra ler
	{
		read(0, comando, 81);
		//printf("comando lido inicio:%s\n", comando);
		if(strcmp(comando,"done")==0) break;
		//criaNovoProcesso(f1, comando);

		/*
			Trazer o codigo da funcao 'criaNovoProcesso' para ca foi a unica maneira que encontrei de me prevenir de um bug
			O que acontece e que quando uma funcao é interrompida, depois ela nao continua de onde parou, ao inves disso ela continua da main.
			Entao se o fork que vai usar a execl para se transformar no processo criado for parado no meio do caminho, depois ele não vai conseguir continuar, e teremos um processo fantasma, que não está executando normalmente.
			Mas se o codigo estiver aqui, mesmo que seja parado entre o printf e a chamada para execl, depois vai continuar do mesmo lugar, assim o bug não acontece (eu espero).		
		*/

		strcpy(arg, comando);
		args[0] = strtok(arg, " ");
		while(TRUE){
			args[i] = strtok(NULL, " ");
			if(args[i]==NULL) break;
			i++;
		}
		id = fork();
		if(id==0){
			printf("%d cria: %d - tempo %d - Novo processo\n%s\n", getppid(), getpid(), tempo, comando);
			execv("./programa", args);
			printf("dafuq");
			encerra(1);
		}	
		FILA_insere(f1,id, 0);
		printf("getpid %d parando processo %d\n", getpid(), id);
		if(id>0)
			kill(id, SIGSTOP);
		else{
			printf("BUG AQUI\n");
		}

		*flag-=1;			
	}
	id = FILA_topId(f1);
	printf("Iniciando processo %d\n", id);
	kill(id, SIGCONT);

	/* Loop para tratar os programas em execução/espera */
	while(TRUE){
		sleep(1);
		tempo++;
		printf("passou 1 u.t. agora estamos em %d, e na fila %d\n", tempo, indexAtual+1);

		/* Atualiza os processos em I/O	*/
		if(!FILA_vazia(filaIO))
			FILA_atualizaIO(filaIO, tempo);
				
		id = FILA_topId(fAtual());
		printf("id= %d \n",id);
		if(id<=0){
			printf("fila atual vazia, procurando outro processo\n");
			if(fReset() == NULL){
				/*Se f reset retorna null então as tres filas estão vazias, e a fila de io tem alguem esperando IO*/
				printf("processo aguardando I/O\n");
				continue;
			}
		}
		tempoRestante = FILA_tempoRestante(fAtual(), tempo);
		printf("tempoRestante:%d do processo %d\n", tempoRestante, id);

		if(solicitouTermino){
			printf("Retirando processo %d (terminado) da fila\n", id);		
			FILA_remove(fAtual());
			fReset();
			solicitouTermino = 0;
			solicitouIO = 0;
		}
		/* Caso um processo tenha extrapolado o tempo, ele deve ser movido para uma fila de nivel mais alto e menor prioridade */
		else if(!solicitouIO && tempoRestante<0 ){
			printf("colocando processo %d numa fila mais alta\n", id);

			if(id>0) kill(id, SIGSTOP);
			else printf("BUG NOUTRO LUGAR\n");

			FILA_remove(fAtual());
			FILA_insere(fProx(), id, tempo);//coloca numa mais alto
		}
		else if(solicitouIO){
			/*
				O processo não extrapolou o tempo de execução dele, vamos checar se ele solicitou IO, e qual era o tempo restante quando ele fez isso.
				Precisamos calcular o tempo restante quando ele pediu IO, e não o tempo restante de agora. */
			tempoRestante = FILA_tempoRestante(fAtual(), FILA_getTempoComecoIO(fAtual()));
			/*
				Caso um processo tenha terminado antes do tempo esperado, ele deve ser movido para uma fila de nivel mais baixo e maior prioridade.
				Temos que colocar o processo que pediu I/O em uma fila de I/O */
			if(tempoRestante >0){
				printf("colocando processo %d numa fila mais baixa\n", id);	
				printf("colocando processo %d em I/O\n", id);
				FILA_comecaIO(fAtual(), fAnt(), filaIO, tempo);	//coloca numa mais baixo			
			}
			/*	Dentro da faixa de 'acabou no tempo certo'
				Caso o processo tenha terminado no tempo correto, ele é movido para o final da fila.
				Temos que colocar o processo que pediu I/O em uma fila de I/O */	
			else if(tempoRestante<=0){
				printf("colocando processo %d no final da mesma fila\n", id);
				printf("colocando processo %d em I/O\n", id);			
				FILA_comecaIO(fAtual(), fAtual(), filaIO, tempo);//coloca na mesma
			}
			solicitouIO = 0;
		}
		/*
			Agora vamos checar se existe algum outro processo numa fila de prioridade maior que está esperando para ser executado.
			Se houver, precisamos trocar da fila atual para a fila dele.
		*/
		/* Fila atual muda para a primeira fila vazia de maior prioridade */
		/* Se alguma fila alem da filaIO tem um processo aguardando */
		if(fReset()!=NULL){
			/* Precisamos continuar o processo atual */
			id = FILA_topId(fAtual());
			kill(id, SIGCONT);
		}


	}

	/* Encerrando... */
	encerra(0);
	return 0;
}



/* Funcoes de controle de processos */

/*
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
		printf("criado:%d por: %d - tempo %d - Novo processo id\n%s\n", getpid(), getppid(), tempo, comando);
		execv("./programa", args);
		printf("dafuq");
		encerra(1);
	}	
	FILA_insere(f,id, 0);
	printf("getpid %d parando processo %d\n", getpid(), id);
	if(id>0)
		kill(id, SIGSTOP);
	else{
		printf("BUG AQUI\n");
	}
}
*/

void processoIO(int sinal){
	int id = FILA_topId(fAtual());
	printf("Processo %d solicitou IO em tempo %d\n", id, tempo);
	if(id>0){
		FILA_setTempoComecoIO(fAtual(), tempo);
		kill(id, SIGSTOP);
		solicitouIO = 1;
	}
}

void processoTermina(int sinal){
	int id;
	id = FILA_topId(fAtual());
	printf("processo %d solicitouTermino em %d\n", id, tempo);
	solicitouTermino = 1;
}

/* Funcoes de controle de fila */

ptFila fAtual(){
	//printf("funcFAtual indexAtual: %d\n", indexAtual);
	if(indexAtual<0 || indexAtual>2) return NULL;
	return filas[indexAtual];
}	

/* 
	Retorna primeira fila nao vazia, atribui index a indexAtual.
	Retorna NULL se nenhuma fila estiver vazia, atribui -1 a indexAtual.
*/
ptFila fReset(){
	
	int i;
	printf("funcFreset indexAtual de: %d\n", indexAtual);
		
	for(i=0;i<NFILAS;i++){
		if(!FILA_vazia(filas[i])){
			indexAtual = i;	
			printf("funcFreset indexAtual para: %d\n", indexAtual);
			return filas[i];
		}
	}

	if(FILA_vazia(filaIO))
		encerra(0);
	
	return NULL;
}



/* Retorna fila, um nivel de fila maior do que a atual (note que o nivel de fila é inversamente proporcional a prioridade) */
ptFila fProx(){
	int i =  indexAtual+1;
	//printf("funcFprox indexAtual: %d\n", indexAtual);
	if(i>NFILAS-1) i = NFILAS-1;
	return filas[i];
}

/* Retorna fila, um nivel de fila menor do que a atual (note que o nivel de fila é inversamente proporcional a prioridade) */
ptFila fAnt(){
	int i = indexAtual-1;
	//printf("funcFant indexAtual: %d\n", indexAtual);
	if (i<0) i = 0;
	return filas[i];
}

void encerra(int status){
	printf("encerrando\n");
	shmdt(flag);
	shmctl(segmento, IPC_RMID, 0);
	FILA_libera(f1);
	FILA_libera(f2);
	FILA_libera(f3);
	FILA_libera(filaIO);
	exit(status);
}
