#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <fcntl.h>


//só pode ter 10 argumentos
#define MAX_ARGS 10
#define MAX_STRING 80
#define TRUE 1
#define CHAVE 8180

int segmento, *flag;

void encerra(int status);

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
	int id;
	char comando[MAX_STRING];
	int fd[2];
	
	//signal(SIGCHLD, encerra);

	segmento = shmget(CHAVE, sizeof(int), IPC_CREAT | S_IRUSR | S_IWUSR);
	flag= (int*) shmat (segmento,0,0);
	*flag=0;
	if(pipe(fd)<0){
		printf("erro! ao criar pipe\n");
		exit(1);
	}	
	

	//printf("Digite a quantidade de programas que devem ser criados no início.\n");
	//scanf(" %d", &i);
	*flag=1;
	
	/* O processo escalonador sera filho deste processo */
	id = fork();
	if(id==0){
		close(fd[1]);//fechando o canal de escrita
		
		if(dup2(fd[0],0)==-1)
		{
			perror("Erro mudando o stdin do filho\n");
			return -1;
		}
		
		/*
		if(dup2(saida,1)==-1)
		{
			perror("Erro mudando o stdout do filho\n");
			return -1;
		}
		*/
		execl("./mainF", "/mainF", NULL);
		printf("ainda estou aqui nao dei exec\n");		
		exit(-1);
	}
	else{
		close(fd[0]);
		
		/* Loop para ler os comandos */
		
		
		do{

			printf("Use './prog a b c..' e 'done' para terminar.\n");		
	
			scanf(" %80[^\n]", comando);
			//printf("comando digitado A %s\n",comando);
			*flag+=1;
			write(fd[1], comando, strlen(comando)+1);
			
	
		}while(strcmp("done", comando));
		*flag = 0;
/*
		do{

			printf("Use './prog a b c..' e 'exit' para terminar.\n");		
	
			scanf(" %s\n", comando);
			printf("comando digitado B %s\n",comando);	
	
			*flag+=1;
			write(fd[1], comando, strlen(comando)+1);
			printf("flag:%d", *flag);
			

	
		}while(strcmp("exit", comando));
*/
		waitpid(id, NULL, 0);
		while(1);		
		printf("aguardando escalonador terminar\n");
		printf("liberando flag \n");
		// libera a memória compartilhada do processo
		shmdt(flag);
		printf("liberando flag \n");
		// libera a memória compartilhada
		shmctl(segmento, IPC_RMID, 0);
		printf("fechando saida  pelo pipe\n");
		close(fd[1]);
	}
	encerra(0);
	return 0;
}

void encerra(int status){
	printf("\t\tfilho encerrou\n");
	shmdt(flag);
	shmctl(segmento, IPC_RMID, 0);
	exit(status);
}
