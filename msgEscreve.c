#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/wait.h>

#define MENSAGEM_MAX 80
#define CHAVE 8180

int main ( int argc, char *argv[]){
	int segmento, i;
	char *mensagem;
	// aloca a memória compartilhada
	segmento = shmget (IPC_PRIVATE, MENSAGEM_MAX*sizeof(char), IPC_CREAT |  S_IRUSR |S_IWUSR);// associa a memória compartilhada ao processo
	mensagem = (char*) shmat (segmento, 0, 0);// comparar o retorno com -1

	printf("%d\n", segmento);

	if(segmento<0)printf("erro\n");

	scanf(" %[^\n]", mensagem);
	printf("%s\n", mensagem);
	for(i=90;i>5;i--){
		printf("%d\n", i);
		sleep(1);
	}
	printf("SEU TEMPO ESTÁ ACABANDO!\n");
	for(i=5;i>0;i--){
		printf("%d\n", i);
		sleep(1);
	}
	printf("VAI EXPLODIR!!\n");
	printf("%s\n", mensagem);

	// libera a memória compartilhada do processo
	shmdt(mensagem);
	// libera a memória compartilhada
	//shmctl(segmento, IPC_RMID, 0);
	return 0;
}

