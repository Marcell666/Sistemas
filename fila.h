typedef struct fila Fila;
typedef Fila* ptFila;

ptFila FILA_cria(double tempo);
void FILA_insere(ptFila fila, int id, double tempoAtual);
void FILA_remove(ptFila fila);
void FILA_comecaIO(ptFila fila, double tempoAtual);
int FILA_comecaCPU(ptFila fila, double tempoAtual);
double FILA_tempoRestante(ptFila fila, double tempoAtual);
int FILA_topId(ptFila ptFila);
int FILA_vazia(ptFila fila);
void FILA_limpa(ptFila fila);
void FILA_libera(ptFila fila);




