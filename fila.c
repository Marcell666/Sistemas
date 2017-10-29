#include <stdio.h>
#include <stdlib.h>

#include "fila.h"

/* tempo padrão de chamada I/O */
#define TEMPO_IO 3


/*inserir aqui a fila que pertence?*/
typedef struct elemento{
	struct elemento *prox;
	struct fila *filaOriginal;
	int id;
	/* tempo restante para o processo terminar */
	int tempoRestante;
	/* tempo restante para o io acabar */
	int tempoRestanteIO;
	/* tempo restante quando este processo pediu IO */
	int tempoRestanteComecoIO;

} Elemento;

struct fila{
	Elemento *prim;
	Elemento *ult;
	/* tempo de uso padrao desta fila */
	int tempo;
	int index;
};

ptFila FILA_cria(int tempo, int index){
	ptFila fila = (Fila*) malloc(sizeof(Fila));
	fila->tempo = tempo;
	fila->prim = NULL;
	fila->ult = NULL;
	fila->index = index;
	return fila;
}
void FILA_insere(ptFila fila, int id, int tempoAtual){
	Elemento *el;
	el = (Elemento*) malloc(sizeof(Elemento));
	el->id = id;
	el->tempoRestante = fila->tempo;
	el->tempoRestanteIO = 0;
	el->tempoRestanteComecoIO = 0;
	el->prox = NULL;
	if(FILA_vazia(fila))
		fila->prim = el;
	else
		fila->ult->prox = el;
	fila->ult = el;
}
void FILA_remove(ptFila fila){
	Elemento *remove;
	if(FILA_vazia(fila)) return;
	remove = fila->prim;
	fila->prim = remove->prox;
	if(fila->prim == NULL)
		fila->ult = NULL;
	free(remove);
}
void FILA_comecaIO(ptFila deFila, ptFila paraFila, ptFila filaIO, int tempoAtual){
	int id = deFila->prim->id;	
	
	FILA_remove(deFila);
	FILA_insere(filaIO, id, tempoAtual);
	filaIO->ult->filaOriginal = paraFila;
	filaIO->ult->tempoRestanteIO = TEMPO_IO;
	printf("FILA: processo %d comeca IO em %d deve acabar em %d\n", id, tempoAtual, tempoAtual + TEMPO_IO);
}
void FILA_atualizaIO(ptFila fila, int tempoAtual){
	int id;
	ptFila filaOriginal;
	Elemento *el, *ant=NULL;
	el = fila->prim;
	while (el != NULL){
		el->tempoRestanteIO-=1;
		printf("FILA: processo %d tempo restante IO %d\n", el->id, el->tempoRestanteIO);
		if(el->tempoRestanteIO<=0){
			filaOriginal = el->filaOriginal;
			id = el->id;

			/* Estou removendo do topo, basta usar a remove() */
			if(ant==NULL){
				el = el->prox;
				FILA_remove(fila);
			}

			/* Não estou removendo do topo, não posso usar a remove() */
			else{
				ant->prox = el->prox;
				if(el->prox == NULL)
					fila->ult = ant;
				ant = el;
				el = el->prox;
				free(ant);
			}
			printf("FILA: processo %d acabou IO em %d, indo para fila %d\n", id, tempoAtual, filaOriginal->index+1);
			FILA_insere(filaOriginal, id, tempoAtual);
			
		}
		else{
			ant = el;
			el = el->prox;
		}
	}
			
}
int FILA_atualizaCPU(ptFila fila, int tempoAtual){
	if(FILA_vazia(fila)) return -1;
	fila->prim->tempoRestante -= 1;
	printf("FILA: processo %d atualiza CPU em %d\n", fila->prim->id, tempoAtual);
	return fila->prim->tempoRestante;
}
int FILA_tempoRestante(ptFila fila){
	return fila->prim->tempoRestante;
}
int FILA_topId(ptFila fila){
	if(FILA_vazia(fila)) return -1;
	return fila->prim->id;
}
int FILA_getTempoRestanteComecoIO(ptFila fila){
	if(FILA_vazia(fila)) return -1;
	return fila->prim->tempoRestanteComecoIO;
	return 0;
}

int FILA_getIndex(ptFila fila){
	return fila->index;
}
int FILA_setTempoRestanteComecoIO(ptFila fila){
	if(FILA_vazia(fila)) return -1;
	fila->prim->tempoRestanteComecoIO = fila->prim->tempoRestante;
	return 0;
}
int FILA_vazia(ptFila fila){
	return fila->prim == NULL;
}
void FILA_limpa(ptFila fila){
	Elemento *remove;
	fila->ult = NULL;
	while(!FILA_vazia(fila)){
		remove = fila->prim;
		fila->prim = remove->prox;
		free(remove);
	}
}
void FILA_libera(ptFila fila){
	FILA_limpa(fila);
	free(fila);
}
