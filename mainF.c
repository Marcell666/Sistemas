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

	
	f1 = FILA_cria(1);
	f2 = FILA_cria(2);
	f3 = FILA_cria(4);
	
	filaIO = FILA_cria(666);

	signal(SIGUSR1, processoIO);
	signal(SIGUSR2, processoTermina);
	
	/* Loop para criar os processos */

	while (*flag>0) //tem coisa pra ler
	{
		read(0, comando, 81);
		//printf("comando lido inicio:%s\n", comando);
		if(strcmp(comando,"done")==0) break;
		criaNovoProcesso(f3, comando);
		*flag-=1;			
	}
	indexAtual =2;
	id = FILA_topId(f3);
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
				
		

		//printf("passou 1 u.t. agora estamos em %d\n", tempo);
		id = FILA_topId(fAtual());
		printf("id= %d \n",id);
		if(id<=0){
			printf("fila atual vazia, procurando outro processo\n");
			if(fReset() == NULL  && FILA_vazia(filaIO)){
				printf("todas as filas estao vazia\n");
				//encerra(0);
			}
			else if(!FILA_vazia(filaIO)){
				printf("processo aguardando I/O");
				continue;
				}
			else
				printf("processo ocioso");
		
		}
printf("A1\n");
		tempoRestante = FILA_tempoRestante(fAtual(), tempo);
		printf("tempoRestante:%d do processo %d\n", tempoRestante, id);
	
		/* Se o tempo acabou, ou o programa solicitou I/O, então algum processo deve trocar */
		if(solicitouIO && tempoRestante<0){

			if(id>0)
				kill(id, SIGSTOP);
			else{
				printf("BUG NOUTRO LUGAR\n");
			}
printf("A2\n");
			/* Caso um processo tenha extrapolado o tempo, ele deve ser movido para uma fila de nivel mais alto e menor prioridade */
			if(!solicitouIO && tempoRestante<0 ){
				printf("colocando processo %d numa fila mais alta\n", id);	
				FILA_remove(fAtual());
				FILA_insere(fProx(), id, tempo);//coloca numa mais alto	
			}
		

			id = FILA_topId(fAtual());
			solicitouIO = 0;
printf("A3\n");
			if(id==-1){
				printf("fila vazia\n");
				if(FILA_vazia(filaIO)){
					printf("Nenhum processo restante, fechando o programa");
					encerra(0);
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
printf("A4\n");
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
			kill(id, SIGSTOP);
			/* E continuar o processo atual */
			id = FILA_topId(fAtual());
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

void processoIO(int sinal){
	int tempoRestante;
	int id = FILA_topId(fAtual());
	printf("Processo %d solicitou IO em tempo %d\n", id, tempo);
	kill(id, SIGSTOP);
		
	tempoRestante = FILA_tempoRestante(fAtual(), tempo);
	/* Caso um processo tenha terminado antes do tempo esperado, ele deve ser movido para uma fila de nivel mais baixo e maior prioridade.
		Temos que colocar o processo que pediu I/O em uma fila de I/O */
		/*else*/ if(tempoRestante >0){
			printf("colocando processo %d numa fila mais baixa\n", id);	
			printf("colocando processo %d em I/O\n", id);
			FILA_comecaIO(fAtual(), fAnt(), filaIO, tempo);	//coloca numa mais baixo		
			
		}
		/*	Dentro da faixa de 'acabou no tempo certo'
			Caso o processo tenha terminado no tempo correto, ele é movido para o final da fila 
		Temos que colocar o processo que pediu I/O em uma fila de I/O */	
		else if(tempoRestante<=0){
			printf("colocando processo %d no final da mesma fila\n", id);
			printf("colocando processo %d em I/O\n", id);			
			FILA_comecaIO(fAtual(), fAtual(), filaIO, tempo);//coloca na mesma
	
		}


	if(id>0)
		solicitouIO = 1;
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
	int i;
	printf("Index atual %d\n",indexAtual);
//	if(indexAtual<0 || indexAtual>2) return NULL;
//	return filas[indexAtual];

	switch (indexAtual)
{
   case 0:
     return f1;
   break;
   
   case 1:
     return f2;
   break;
      
   case 2:
     return f3;
   break;
   default: //erro???
     return NULL;
}
//	return filas[i];*/
}	

/* 
	Retorna primeira fila nao vazia, atribui index a indexAtual.
	Retorna NULL se nenhuma fila estiver vazia, atribui -1 a indexAtual.
*/
ptFila fReset(){
	
	int i;
	printf("RA\n");
	if(filas[0]==NULL || filas[1] == NULL)
		printf("filas nulas");
//	indexAtual = 0;
//	for(i=0;i<NFILAS;i++){
//		if(!FILA_vazia(filas[i])){
//		indexAtual = i;			
//		}
//	}
	if (!FILA_vazia(f1))
	{
		indexAtual = 0;
		return f1;
	}
	if (!FILA_vazia(f2))
	{
		indexAtual = 1;
		return f2;
	}
	if (!FILA_vazia(f3))
	{
		indexAtual = 2;
		return f3;
	}	

	printf("RF\n");
	switch (indexAtual)
	{
	   case 0:	   	
		 return f1;
	   
	   
	   case 1:
		 return f2;
	   
		  
	   case 2:
		 return f3;
	   
	   default: //erro???
	   	indexAtual = -1;
		 return NULL;
	}
	
	indexAtual = -1;
	
	return NULL;
	}

/* Retorna fila, um nivel de fila maior do que a atual (note que o nivel de fila é inversamente proporcional a prioridade) */
ptFila fProx(){
	
	//int i;// =  (indexAtual+1)%3;
	//return filas[i];
	switch (indexAtual)
{
   case 0:
   	//indexAtual =1;
     return f2;
   break;
   
   case 1:
     //indexAtual =2;
     return f3;
   break;
      
   case 2:
    // indexAtual =2;
     return f3;
   break;
   default: //erro???
     return NULL;
}
	//indexAtual=i; //aqui que sera o controle?
	return NULL;
	
	
	//return f1;
}

/* Retorna fila, um nivel de fila menor do que a atual (note que o nivel de fila é inversamente proporcional a prioridade) */
ptFila fAnt(){
	
	int i; //= indexAtual-1;
	//if (i<0)i = NFILAS-1;
	printf("FAnt index = %d\n",indexAtual);
	switch (indexAtual)
{
   case 0:
    //indexAtual=0;
   	return f1;

   case 1:
   // indexAtual=0;
   	return f1;
      
   case 2:
  //  indexAtual=1;
   	return f2;
   default: //erro???
     break;
}
	//indexAtual=i; //aqui que sera o controle?
	
	//return filas[i];
	
	return NULL;
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
