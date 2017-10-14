#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/wait.h>

#define MENSAGEM_MAX 80
#define CHAVE 8180

int main ( int argc, char *argv[]){
	int segmento ;
	char *mensagem;
	int chave = atoi(argv[1]);
	
	printf("%d\n", chave);

	// aloca a memória compartilhada
	segmento = shmget (chave, MENSAGEM_MAX*sizeof(char), IPC_EXCL | S_IRUSR |S_IWUSR);// associa a memória compartilhada ao processo
	mensagem = (char*) shmat (segmento, 0, 0);// comparar o retorno com -1

	printf("mensagem de outro programa:\n%s\n", mensagem);

	// libera a memória compartilhada do processo
	shmdt(mensagem);
	// libera a memória compartilhada
	shmctl(segmento, IPC_RMID, 0);
	return 0;
}

