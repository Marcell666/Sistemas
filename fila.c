#include <stdio.h>
#include <stdlib.h>

#include "fila.h"

/* tempo padrão de chamada I/O */
#define TEMPO_IO 3


/*inserir aqui a fila que pertence?*/
typedef struct elemento{
	struct elemento *prox;
	int id;
	/* tempo absoluto no qual o processo deve terminar */
	int tempoTerminoCPU;
	/*  tempo absoluto no qual o IO deve terminar */
	int tempoTerminoIO;
} Elemento;

struct fila{
	Elemento *prim;
	Elemento *ult;
	/* tempo de uso padrao desta fila */
	int tempo;
};

ptFila FILA_cria(int tempo){
	ptFila fila = (Fila*) malloc(sizeof(Fila));
	fila->tempo = tempo;
	fila->prim = NULL;
	fila->ult = NULL;
	return fila;
}
void FILA_insere(ptFila fila, int id, int tempoAtual){
	Elemento *el;
	el = (Elemento*) malloc(sizeof(Elemento));
	el->id = id;
	el->tempoTerminoCPU = tempoAtual+fila->tempo;
	el->tempoTerminoIO = 0;
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
void FILA_comecaIO(ptFila fila, int tempoAtual){
	fila->prim->tempoTerminoIO = tempoAtual+TEMPO_IO;
}
int FILA_comecaCPU(ptFila fila, int tempoAtual){
	if(tempoAtual<fila->prim->tempoTerminoIO) return 1;
	fila->prim->tempoTerminoCPU = tempoAtual+fila->tempo;
	return 0;
}
int FILA_tempoRestante(ptFila fila, int tempoAtual){
	int tempoRestante = fila->prim->tempoTerminoCPU - tempoAtual;
	return tempoRestante;
}
int FILA_topId(ptFila fila){
	if(FILA_vazia(fila)) return -1;
	return fila->prim->id;
}
/*	DEPRECATED TODO Remove*/
//void FILA_topResetTempo(ptFila){
//	if(FILA_vazia(ptFila)) return;
//	ptFila->prim->tempo = ptFila->tempo;
//	return;
//}
//double FILA_topTempo(ptFila ptFila){
//	if(FILA_vazia(ptFila)) return;
//	return ptFila->prim->tempo;
//	return 0.0D;
//}
/* DEPRECATED END */

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
