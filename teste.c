#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>

void f1(int sinal);

int main(int argc, char **argv){
	int tempoAtual;
	tempoAtual = time(NULL);
	
	signal(SIGUSR1, f1);

	while(1)
	while(tempoAtual!=time(NULL)){
		printf("passou um segundo\n");
		tempoAtual = time(NULL);
		sleep(10);
		printf("passou 10\n");
	}
	
	return 0;
}

void f1(int sinal){
	printf("sinal\n");
}

