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

static char *comando;
ptFila f1,f2,f3, fAtual;
ptFila filas[3] = {f1, f2, f3};
int indexAtual;
int tempo;

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
	int i, id, status;
	int segmento;
	int tempo=0;
	FILA_cria(f1, 1);
	FILA_cria(f2, 2);
	FILA_cria(f3, 4);
	tempo = 0;
	indexAtual = -1;

	/* Criando variável compartilhada, vou usá-la para passar os comandos lidos aqui para o escalonador */
	segmento = shmget(CHAVE, MAX_STRING*sizeof(char), IPC_EXCL | S_IRUSR |S_IWUSR);
	comando = shmat(segmento, NULL, NULL);

	/* Usando sinais para cuidar dos programas novos */
	signal(SIGUSR1, criaNovoProcesso());
	signal(SIGUSR2, processoIO());
	signal(SIGCHLD, processoTermina());

	/* Loop para tratar os programas em execução/espera */
	do{
		printf("passou 1 u.t. agora estamos em %d\n", tempo);
		/* Caso um processo tenha extrapolado o tempo, ele deve ser movido para uma fila de nivel mais baixo */
		if(FILA_tempoRestante(fAtual()) < LIM_MIN){
			id = FILA_topId(fAtual());
			FILA_remove(fAtual());
			FILA_insere(fAnt(), id);
		}
	}

	else if(restante>LIM_MAX){
	/* dentro da faixa de 'processo terminou no tempo certo' */

		FILA_atualiza(fAtual);
		sleep(1);
		tempo++;
	}while(strcmp(comando, "exit") != 0);	

	/* Encerrando... */
	shmdt(comando);
	shmctl(segmento);
	FILA_libera(f1);
	FILA_libera(f2);
	FILA_libera(f3);
	return 0;
}

/* Funcoes de controle de processos */

void criaNovoProcesso(int sinal){
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
	FILA_insere(f1,id);
	if(fAtual() != f1)
		FILA_comecaCPU(f1, clock());
	/* fila atual passa a ser f1 */
	fReset();
	/*
		Estou assumindo que caso um processo A na fila 2 esteja sendo executado e seja interrompido para dar lugar a um novo processo na B fila 1, não há necessidade de resetar o quantum do processo A. De modo que quando todos os processos na fila sejam esgotados, voltaremos a executar o processo A somente pelo quantum restante. 
	*/

	/*
		Fazemos o leitor continuar a executar. Ele havia parado ate que o escalonador pudesse criar o novo processo.
	*/
	kill(getppid(), SIGCONT);
}

void processoIO(int sinal){
	/*
		Este processo é que chamou o escalonador, então ele fez isso enquanto estava executando e não depois de seu tempo acabar.
		Nos resta saber se ele parou junto com o tempo do escalonador ou antes disso.
	*/
	int id;
	int restante = FILA_atualizaCPU(fAtual, clock());
	/* dentro da faixa de 'mudar prioridade para cima' */
	if(restante>LIM_MAX){
		id = FILA_topId(fAtual);
		FILA_remove();
		FILA_insere(fNext)			
	}
	/* dentro da faixa de 'acabou no tempo certo' */
	else if(restante>LIM_MIN && restante<=LIM_MAX){
	
	}
	/*
		dentro da faixa mudar prioridade para baixo
		Noite que isso na verdade não deve acontecer, pois se passar do tempo o proprio escalonador é que deve parar o processo como na funcao acima. Mas isso é so para garantir, assim mesmo que por alguma razão os processos percam a sincronização, isso não terá um grande impacto.	
	*/
	else{
		
	}
}

void processoTermina(int sinal){
	printf("processo %d terminado\n", FILA_topId(fAtual));
	FILA_remove(fAtual);
}

/* Funcoes de controle de fila */

ptFila fAtual(){
	if(indexAtual<0 ||indexAtual>2) return NULL;
	return filas[indexAtual];
}

/* 
	Retorna primeira fila nao vazia, atribui index a indexAtual.
	Retorna NULL se nenhuma fila estiver vazia, atribui -1 a indexAtual.
*/
ptFila fReset(){
	int i;
	indexAtual = 0;
	for(i=0;i<NFILAS;i++){
		if(!FILA_vazia(filas[i])){
			indexAtual = i;
			return filas[indexAtual];
		}
	}
	indexAtual = -1;
	return NULL;
}

/* Retorna fila, um nivel de prioridade maior do que a atual */
ptFila fProx(){
	int i =  (indexAtual+1)%3;
	return filas[i];
}

/* Retorna fila, um nivel de prioridade menor do que a atual */
ptFila fAnt(){
	int i = indexAtual-1;
	if (i<0)i = NFILAS-1;
	return filas[i];
}
