typedef struct fila Fila;
typedef Fila* ptFila;

ptFila FILA_cria(int tempo);
void FILA_insere(ptFila fila, int id, int tempoAtual);
void FILA_remove(ptFila fila);
void FILA_comecaIO(ptFila fila, int tempoAtual);
int FILA_comecaCPU(ptFila fila, int tempoAtual);
int FILA_tempoRestante(ptFila fila, int tempoAtual);
int FILA_topId(ptFila ptFila);
int FILA_vazia(ptFila fila);
void FILA_limpa(ptFila fila);
void FILA_libera(ptFila fila);
