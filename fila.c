#include <stdio.h>
#include <stdlib.h>

/* tempo padrão de chamada I/O */
#define TEMPO_IO 3.0D

typedef struct elemento{
	struct elemento *prox;
	int id;
	/* tempo relativo que este processo ainda deve executar antes que escalonador o troque por outro*/
	double tempoRestante;	
	/* tempo absoluto no ultimo momento em que o tempo restante foi atualizado */
	double tempoUltimo;
	/*  tempo absoluto no qual o IO deve terminar */
	double tempoIO;
} Elemento;

struct fila{
	Elemento *prim;
	Elemento *ult;
	/* tempo de uso padrao desta fila */
	double tempo;
};

ptFila FILA_cria(double tempo){
	ptFila = (Fila*) malloc(sizeof(Fila));
	ptFila->tempo = tempo;
	ptFila->prim = NULL;
	ptFila->ult = NULL;
}
void FILA_insere(ptFila ptFila, int id, tempoAtual){
	Elemento el;
	el = (Elemento*) malloc(sizeof(Elemento));
	el->id = id;
	el->tempoRestante = ptFila->tempo;
	el->tempoUltimo = tempoAtual;
	el->tempoIO = 0.0D;
	el->prox = NULL;
	if(FILA_vazia(ptFila))
		ptFila->prim = el;
	else
		ptFila->ult->prox = el;
	ptFila->ult = el;
}
void FILA_remove(ptFila ptFila){
	Elemento *remove;
	if(FILA_vazia(ptFila)) return;
	remove = ptFila->prim;
	ptFila->prim = remove->prox;
	if(ptFila->prim == NULL)
		ptFila->ult = NULL;
	free(remove);
}
void FILA_comecaIO(ptFila, double tempoAtual){
	ptFila->prim->tempoIO = tempoAtual+TEMPO_IO;
}
int FILA_comecaCPU(ptFila, double tempoAtual){
	if(tempoAtual<ptFila->prim->tempoIO) return 1;
	ptFila->prim->tempoUltimo = tempoAtual;
	return 0;
}
double FILA_atualizaCPU(ptFila, double tempoAtual){
	ptFila->prim->tempoRestante -= tempoAtual-ptFila->prim->tempoUltimo;
	ptFila->prim->tempoUltimo = tempoAtual;
	return ptFila->prim->tempoRestante;
}
/*	DEPRECATED TODO Remove*/
int FILA_topId(ptFila ptFila){
	if(FILA_vazia(ptFila)) return;
	return ptFila->prim->id;
}
void FILA_topResetTempo(ptFila){
	if(FILA_vazia(ptFila)) return;
	ptFila->prim->tempo = ptFila->tempo;
}
double FILA_topTempo(ptFila ptFila){
	if(FILA_vazia(ptFila)) return;
	return ptFila->prim->tempo;
}
/* DEPRECATED END */

int FILA_vazia(ptFila ptFila){
	return ptFila->prim == NULL;
}
void FILA_limpa(ptFila ptFila){
	Elemento *remove;
	ptFila->ult = NULL;
	while(!FILA_vazia(ptFila)){
		remove = ptFila->prim;
		ptFila->prim = remove->next;
		free(remove);
	}
}
void FILA_libera(ptFila ptFila){
	FILA_limpa(ptFila);
	free(ptFila);
}

