#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "fila.h"

//só pode ter 10 argumentos
#define MAX_ARGS 10
#define MAX_STRING 80
#define TRUE 1
#define CHAVE 8180
#define LIM_MAX 0.5D
#define LIM_MIN -0.5D

void criaNovoProcesso(int sinal);
void processoTermina(int sinal);

ptFila f1,f2,f3;
ptFila filas[3] = {f1, f2, f3};
int indexAtual;
double tempo;

/*

	Acho que estou tendo um problema quanto a precisão de tempo.
	Existem tres situações envolvendo o tempo do escalonador e momento em que o processo pede IO
	Se o processo pede IO depois que o tempo do escalonador acaba, o que vai acontecer primeiro é que o escalonador vai parar o processo, e aumentar o nivel dele, (diminuir a prioridade).
	Se o processo pede IO antes que o tempo do escalonador acabe, o que vai acontecer primeiro é o processo enviar um sinal para o escalonador, que vai ver que ele pediu IO, vai coloca-lo num nivel superior e vai passar para o proximo processo.
	Mas quando é entao que o processo NÃO sera mudado de fila? Quando ele para junto com o escalonador.
	Precisamos considerar o tempo como uma faixa e não como um instante. Assim o escalonador vai esperar o processo se atrasar para pará-lo, e programa quando o programa se adiantar, o escalonador vai checar o proprio tempo antes de mudar de fila.

	Usando dupla precisão posso estabelecer faixas de tempo, vamos usar a fila que tem 4 unidades como exemplo.
	O escalonador possui uma margem de erro, se o processo pede IO, depois de ter executado de 3.5 ate 4.5 ut, entao o escalonador "aproxima" isso para 4 ut. Se o processo pede IO antes de ter executado 3.5 ut então o processo é trocado de fila. Se o escalonador ate 4.5 ut executados, não recebe sinal do processo, então quer dizer que ele esta atrasado e esera trocado de fila.

	No caso a fila de 4 ut, é a de maior nivel, então um processo que extrapolasse e ja estivesse dentro dela, não seria trocado, mas vale o exemplo.

*/


int main(int argc, char **argv){
	int i;
	char comando[MAX_STRING];
	ptFila f1 = FILA_cria(1);
	ptFila f2 = FILA_cria(2);
	ptFila f3 = FILA_cria(4);
	tempo = 0;
	indexAtual = -1;

	/* Usando sinais para cuidar dos programas novos */
	//signal(SIGUSR1, processoIO());
	signal(SIGCHLD, processoTermina());
	
	/* 
		Já contamos que o programa leCmd redirecionou a entrada e saida padrão deste processo,
		Então podemos usar scanf e printf para nos comunicar com ele.
	*/
	scanf("%d", &i);
	printf(	"%d inteiro: %d\n", getpid(),i);

	/* Loop para criar os processos */
	for(;i>0;i--){
		scanf("%s", comando);
		criaNovoProcesso(f1, comando);
	}

	/* Loop para tratar os programas em execução/espera */
	while(TRUE){
		if(tempo-clock()<1000) continue;
		tempo = clock();
		printf("passou 1 u.t. agora estamos em %lf\n", tempo);
	}

	FILA_libera(f1);
	FILA_libera(f2);
	FILA_libera(f3);
	return 0;
}

/* Funcoes de controle de processos */

void criaNovoProcesso(Fila f, char *comando){
	int id=0;
	int i=0;
	char args[MAX_ARGS][MAX_STRING];
	printf("Novo processo a ser criado %d\n", sinal);
	while(TRUE){
		args[i] = strtok(comando, " ");
		if(args[i]==NULL) break;
		i++;
	}
	id = fork();
	if(id==0)
		execv("./programa", args);
	FILA_insere(f,id);
}

void processoTermina(int sinal){
	printf("processo %d terminado\n", FILA_topId(fAtual));
	FILA_remove(fAtual);
}

/* Funcoes de controle de fila */

ptFila fAtual(){
	return f1;
}
