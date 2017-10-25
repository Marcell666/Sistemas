#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
int main (int argc, char *argv[])
{
	int segmento;
	
	int* flag;
	char msg[81];
	
	
	
	
	// aloca a memória compartilhada
	segmento = shmget (8182, sizeof (int), S_IRUSR);
	
	// associa a memória compartilhada ao processo
	flag = (int*) shmat (segmento, 0, 0); // comparar o retorno com -1
	
	
	while (1)
	{
		
		if(*flag==1)
		{
		
			printf("flag %d\n",*flag);
			scanf (" %80[^\n]",msg);
			*flag=0;
			printf("sou filho e a msg lida foi Mensagem lida: %s\n", msg);
		}
		else if (*flag==-1)
			break;

	}
	
	// libera a memória compartilhada do processo
	shmdt(flag);
	// libera a memória compartilhada pai libera
	//shmctl(segmento, IPC_RMID, 0);
	return 0;
}
