typedef struct fila Fila
typedef Fila* ptFila;

ptFila FILA_cria(int tempo);
void FILA_insere(ptFila ptFila, int id);
void FILA_remove(ptFila ptFila);
void FILA_comecaIO(ptFila, double tempoAtual);
int FILA_comecaCPU(ptFila, double tempoAtual);
double FILA_atualizaCPU(ptFila, double tempoAtual);
int FILA_vazia(ptFila ptFila);
void FILA_limpa(ptFila ptFila);
void FILA_libera(ptFila ptFila);

