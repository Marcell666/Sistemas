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

char *fgets_wrapper(char *buffer, size_t buflen, FILE *fp)
{/*peguei essa funcao da internet porque o gets tava me dando warnings*/
    if (fgets(buffer, buflen, fp) != 0)
    {
        size_t len = strlen(buffer);
        if (len > 0 && buffer[len-1] == '\n')
            buffer[len-1] = '\0';
        
        return buffer;
    }
    return 0;
}


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
	int i,j, id;
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
	
	//printf("Digite a quantidade de programas que devem ser criados no início.\n");
	//scanf(" %d", &i);
	*flag=2;
	
	/* O processo escalonador sera filho deste processo */
	id = fork();
	if(id==0){
		close(fd[1]);//fechando o canal de escrita
		//char a;
		//char msg[88];	
		
		if(dup2(fd[0],0)==-1)
		{
			perror("Erro mundando o stdin do filho\n");
			return -1;
		}
		//close(fd[0]);
		
		execl("./mainF", "/mainF", NULL);
		//execl("./fil", "/fil", NULL);
		printf("ainda estou aqui nao dei exec\n");		
		exit(-1);
	}
	else{
		close(fd[0]);
		
		/* Loop para ler os comandos */
		
		
		i=2;
		for (j=0;j<i;j++){

			printf("Use 'prog a b c..'.\n");		
			//fflush(stdin);
			
			fgets_wrapper(comando,79,stdin);
			
			//printf("%d - comando digitado: %s\n", fd[1], comando);
			

			// if(contaEspacos(comando) > MAX_ARGS){
			// 	printf("maximo de %d argumentos\n", MAX_ARGS);
			 //	continue;
			// }		
			
			aux=write(fd[1], comando, strlen(comando)+1);
			*flag=1;
					
			//printf("escrito no pipe %d\n", aux);
			
			}
		
		
		//*flag=-1;
		printf("aguardando escalonador terminar\n");
		waitpid(-1,&aux,0);
		//printf("sigint id=%d \n",id);
		//kill(id,SIGINT);
		printf("liberando flag \n");
		// libera a memória compartilhada do processo
		shmdt(flag);
		printf("liberando flag \n");
		// libera a memória compartilhada (esta sendo feito no outro processo)
		shmctl(segmento, IPC_RMID, 0);
		printf("fechando fd[1] \n");
		close(fd[1]);
		return 0;
		}
}
