typedef struct fila Fila
typedef Fila* ptFila;

ptFila FILA_cria(int tempo);
void FILA_insere(ptFila ptFila, int id);
void FILA_remove(ptFila ptFila);
void FILA_atualiza(ptFila);
int FILA_topId(ptFila ptFila);
int FILA_topTempo(ptFila ptFila);
int FILA_vazia(ptFila ptFila);
void FILA_limpa(ptFila ptFila);
void FILA_libera(ptFila ptFila);

