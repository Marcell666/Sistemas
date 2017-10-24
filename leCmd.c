#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>

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
	int i, id;
	char comando[MAX_STRING];
	int segmento, *flag;
	int fd[2];
	int aux;

	segmento = shmget(8180,sizeof(int), IPC_CREAT | S_IRUSR | S_IWUSR);
	flag= (int*) shmat (segmento,0,0);
	*flag=0;
	if(pipe(fd)){
		printf("erro! ao criar pipe\n");
		exit(1);
	}
	

	printf("Digite a quantidade de programas que devem ser criados no início.\n");
	scanf("%d", &i);

	/* O processo escalonador sera filho deste processo */
	id = fork();
	if(id==0){
		close(fd[1]);//fechando o canal de escrita
		//close(1);
		if(dup2(0,fd[0])==-1)
		{
			perror("Erro mundando o stdin do filho");
			return -1;
		}
		
		//execl("./escalonador", "/escalonador", NULL);
		execl("./fil", "/fil", NULL);
	}
	
	close(fd[0]);

	//write(fd[1], "%d", i); 

	/* Loop para ler os comandos */

	for (;i>0;i--){

		printf("Use 'prog a b c..'.\n");		
		fgets_wrapper(comando,80,stdin);
		//printf("%d - comando digitado:\n%s\n", getpid(), comando);
		

		// if(contaEspacos(comando) > MAX_ARGS){
		// 	printf("maximo de %d argumentos\n", MAX_ARGS);
		// 	continue;
		// }

		aux=write(fd[1], comando, strlen(comando)+1);
		//printf("escrevi para o filho %d chars\n",aux);
		*flag=1;
		sleep(2);
	}
	kill(id,SIGINT);
	// libera a memória compartilhada do processo
	shmdt(flag);
	// libera a memória compartilhada (esta sendo feito no outro processo)
	shmctl(segmento, IPC_RMID, 0);
	return 0;
}
