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
#define NFILAS 1
#define CHAVE 8180

/*Podiamos criar uma estrutura pra cada processo com seu pid, tempo de i/o e fila que pertence, assim conseguimos colocar ele em uma fila de i/o saber quanto tempo ele tem que ficar lá e a hora de retirar, alem disso podemos guardar o tempo de execução que ele fez (já está na fila) */

ptFila filaIO;
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
int indexAtual;
int* flag;
int segmento;

int main(int argc, char **argv){
	int tempoRestante;
	int id;
	char comando[81];
	ptFila filaTemp;
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

	f1 = FILA_cria(2);
	//f1 = FILA_cria(1);
	//f2 = FILA_cria(2);
	//f3 = FILA_cria(4);
	filaIO = FILA_cria(666);

	signal(SIGUSR1, processoIO);
	signal(SIGUSR2, processoTermina);
	
	/* Loop para criar os processos */

	while (*flag>0) //tem coisa pra ler
	{
		read(0, comando, 81);
		//printf("comando lido inicio:%s\n", comando);
		if(strcmp(comando,"done")==0) break;
		criaNovoProcesso(f1, comando);
		*flag-=1;			
	}

	id = FILA_topId(fAtual());
	printf("Iniciando processo %d\n", id);
	kill(id, SIGCONT);

	/* Loop para tratar os programas em execução/espera */
	while(TRUE){
		sleep(1);
		tempo++;
		/*
		if (*flag==1) //tem coisa pra ler
		{
			read(0, comando, 81);
			printf("comando lido no meio:%s\n", comando);
			criaNovoProcesso(f1, comando);
			*flag=0;			
		}
		*/
		printf("passou 1 u.t. agora estamos em %d\n", tempo);
		if(!FILA_vazia(filaIO))
			FILA_atualizaIO(filaIO, tempo);
				
		id = FILA_topId(fAtual());
		if(id<=0){
			printf("fila atual vazia, procurando outro processo\n");
			if(fReset() == NULL  && FILA_vazia(filaIO)){
				printf("todas as filas estao vazias\n");
				//encerra(0);
			}
			else if(!FILA_vazia(filaIO))
				printf("processo aguardando I/O\n");
			else
				printf("processo ocioso\n");	
		}		

		tempoRestante = FILA_tempoRestante(fAtual(), tempo);
		printf("tempoRestante:%d do processo %d\n", tempoRestante, id);
		
		/* Se o tempo acabou, ou o programa solicitou I/O, então algum processo deve ser trocado */
		if(solicitouIO || tempoRestante<0){

			if(solicitouIO)
				tempoRestante = FILA_tempoRestante(fAtual(), FILA_getTempoIO(fAtual()));

			if(id>0)
				kill(id, SIGSTOP);
			else
				printf("BUG NOUTRO LUGAR\n");

			/* Caso um processo tenha extrapolado o tempo, ele deve ser movido para uma fila de nivel mais alto e menor prioridade */
			if(!solicitouIO && tempoRestante<0 ){
				printf("colocando processo %d numa fila mais alta\n", id);	
				FILA_remove(fAtual());
				FILA_insere(fProx(), id, tempo);//coloca numa mais alto	
			}
			/* Caso um processo tenha terminado antes do tempo esperado, ele deve ser movido para uma fila de nivel mais baixo e maior prioridade.
			Temos que colocar o processo que pediu I/O em uma fila de I/O */
			else if(solicitouIO && tempoRestante >0){
				printf("colocando processo %d numa fila mais baixa\n", id);	
				printf("colocando processo %d em I/O\n", id);
				FILA_comecaIO(fAtual(), fAnt(), filaIO, tempo);//coloca numa mais baixo	
			}
			/*	Dentro da faixa de 'acabou no tempo certo'
				Caso o processo tenha terminado no tempo correto, ele é movido para o final da fila 
			Temos que colocar o processo que pediu I/O em uma fila de I/O */	
			else if(solicitouIO && tempoRestante<=0){
				printf("colocando processo %d no final da mesma fila\n", id);
				printf("colocando processo %d em I/O\n", id);			
				FILA_comecaIO(fAtual(), fAtual(), filaIO, tempo);//coloca na mesma
			}
		}

		/*
			Agora vamos checar se existe algum outro processo numa fila de priorade maior que está esperando para ser executado.
			Se houver, precisamos trocar da fila atual para a fila dele.
		*/
		/* Fila atual muda para a primeira fila vazia de maior prioridade */
		filaTemp = fAtual();	
		/* Se de fatos estamos em outra fila*/
		if(fReset() != filaTemp){
			printf("pula para a fila de cima\n");
			/* Precisamos parar o processo que estava rodando antes */
			id = FILA_topId(filaTemp);
			if(id>0)
				kill(id, SIGSTOP);
			/* E continuar o processo atual */
			id = FILA_topId(fAtual());
			if(id>0)
				kill(id, SIGCONT);
		}


	}

	/* Encerrando... */
	encerra(0);
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

void processoIO(int sinal){
	int id = FILA_topId(fAtual());
	printf("Processo %d solicitou IO em tempo %d\n", id, tempo);
	solicitouIO = 1;
	FILA_setTempoIO(fAtual(), tempo);
}

void processoTermina(int sinal){
	int id;
	printf("processo %d terminado\n", FILA_topId(fAtual()));
	id = FILA_topId(fAtual());
	FILA_remove(fAtual());
		
	//TODO colocar f2 e f3 tambem
	if(FILA_vazia(filaIO) && FILA_vazia(f1) && FILA_vazia(f2) && FILA_vazia(f3)){
		printf("todos os processos encerraram, terminando programa\n");
		encerra(0);
	}
	if(id>0)
		kill(id, SIGCONT);
	else{
		printf("BUG ALI\n");
	}
}

/* Funcoes de controle de fila */

ptFila fAtual(){
	/*
	if(indexAtual<0 || indexAtual>2) return NULL;
	return filas[indexAtual];
	*/
	return f1;
}

/* 
	Retorna primeira fila nao vazia, atribui index a indexAtual.
	Retorna NULL se nenhuma fila estiver vazia, atribui -1 a indexAtual.
*/
ptFila fReset(){
	/*
	int i;
	indexAtual = 0;
	for(i=0;i<NFILAS;i++){
		if(!FILA_vazia(filas[i])){
			indexAtual = i;
			return filas[indexAtual];
		}
	}
	indexAtual = -1;
	return NULL;
	*/
	return f1;
}

/* Retorna fila, um nivel de fila maior do que a atual (note que o nivel de fila é inversamente proporcional a prioridade) */
ptFila fProx(){
	/*
	int i =  indexAtual+1;
	if (i>=NFILAS-1)i = NFILAS-1;
	return filas[i];
	*/
	return f1;
}

/* Retorna fila, um nivel de fila menor do que a atual (note que o nivel de fila é inversamente proporcional a prioridade) */
ptFila fAnt(){
	/*
	int i = indexAtual-1;
	if (i<0)i = 0;
	return filas[i];
	*/
	return f1;
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
