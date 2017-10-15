#include <stdio.h>
#include <stdlib.h>


typedef struct elemento{
	struct elemento *prox;
	int id;
	int tempo;
} Elemento;

struct fila{
	Elemento *prim;
	Elemento *ult;
	int tempo;
};

ptFila FILA_cria(int tempo){
	ptFila = (Fila*) malloc(sizeof(Fila));
	ptFila->tempo = tempo;
	ptFila->prim = NULL;
	ptFila->ult = NULL;
}
void FILA_insere(ptFila ptFila, int id){
	Elemento el;
	el = (Elemento*) malloc(sizeof(Elemento));
	el->id = id;
	el->tempo = ptFila->tempo;
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
void FILA_atualiza(ptFila){
	if(FILA_vazia(ptFila)) return;
	ptFila->prim->tempo -= 1;
}
int FILA_topId(ptFila ptFila){
	if(FILA_vazia(ptFila)) return;
	return ptFila->prim->id;
}
void FILA_topResetTempo(ptFila){
	if(FILA_vazia(ptFila)) return;
	ptFila->prim->tempo = ptFila->tempo;
}
int FILA_topTempo(ptFila ptFila){
	if(FILA_vazia(ptFila)) return;
	return ptFila->prim->tempo;
}
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

