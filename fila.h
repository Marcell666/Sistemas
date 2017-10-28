typedef struct fila Fila;
typedef Fila* ptFila;

ptFila FILA_cria(int tempo, int index);
void FILA_insere(ptFila fila, int id, int tempoAtual);
void FILA_remove(ptFila fila);

void FILA_comecaIO(ptFila deFila, ptFila paraFila, ptFila filaIO, int tempoAtual);
void FILA_atualizaIO(ptFila fila, int tempoAtual);



int FILA_atualizaCPU(ptFila fila, int tempoAtual);
int FILA_tempoRestante(ptFila fila);
int FILA_getTempoRestanteComecoIO(ptFila fila);
int FILA_setTempoRestanteComecoIO(ptFila fila);


int FILA_getIndex(ptFila fila);
int FILA_topId(ptFila ptFila);
int FILA_vazia(ptFila fila);
void FILA_limpa(ptFila fila);
void FILA_libera(ptFila fila);
