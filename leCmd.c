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
	int i, id, fileid;
	char comando[MAX_STRING];
	int segmento, *flag;
	int fd[2];
	int aux;
	
	segmento = shmget(8182,sizeof(int), IPC_CREAT | S_IRUSR | S_IWUSR);
	flag= (int*) shmat (segmento,0,0);
	*flag=0;
	if(pipe(fd)<0){
		printf("erro! ao criar pipe\n");
		exit(1);
	}
	
	
	/* O processo escalonador sera filho deste processo */
	id = fork();
	if(id==0){
		fileid=open("saida.txt",O_RDWR|O_CREAT|O_TRUNC,0666);
		if(fileid < 0)
		{
			printf("erro ao abrir saida\n");
			exit(-1);
		}
		
		//close(fd[1]);//fechando o canal de escrita
		
		
		if(dup2(fd[0],0)==-1)
		{
			perror("Erro mundando o stdin do filho");
			return -1;
		}
		
		if(dup2(fileid,1)==-1)
		{
			perror("Erro mundando o stdou do filho pra arquivo");
			return -1;
		}
		
		
		//execl("./escalonador", "/escalonador", NULL);
		execl("./fil", "/fil", NULL);
		
	}
	else{
		//close(fd[0]);

		/* Loop para ler os comandos */
		
		printf("Digite a quantidade de programas que devem ser criados no início.\n");
		scanf("%d", &i);

		for (aux=0;aux<i;aux++){

			printf("Use 'prog a b c..'.\n");		
		
			scanf(" %80[^\n]",comando);
			
			printf("%d - comando digitado:\n%s\n", fd[1], "alo");
			

			 if(contaEspacos(comando) > MAX_ARGS){
			 	printf("maximo de %d argumentos\n", MAX_ARGS);
			 	continue;
			 }

			write(fd[1], comando, strlen(comando)+1);
			*flag=1;
			
			
			}
		
		
		*flag=-1;
		sleep(3);
		kill(id,SIGINT);
		
		// libera a memória compartilhada do processo
		shmdt(flag);
		// libera a memória compartilhada (esta sendo feito no outro processo)
		shmctl(segmento, IPC_RMID, 0);
		close(fd[1]);close(fd[2]);
		return 0;
		}
}
