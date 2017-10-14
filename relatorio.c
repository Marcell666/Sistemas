Bruce Marcellino – 1613172

QUESTÃO 1) Faça um programa para somar matrizes de
acordo com o seguinte algoritmo
	O primeiro processo irá criar duas matrizes
preenchidas e uma terceira vazia em 3 áreas
de memória compartilhada.
	O seu programa deverá gerar um processo
para o cálculo de cada linha da matriz
solução.
	O processo pai deve imprimir a matriz
solução.
-----------------------------------------------------
ARQUIVO.C
#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/wait.h>

#define M_SIZE 9
#define M_LINHA 3
#define N_FILHO 3

void exibeMatriz(int *m, int linhas, int colunas){
	int i, e;
	for(i=0;i<linhas;i++){
		printf("|");
		for(e=0;e<colunas;e++)
			printf("\t%d", m[i*linhas+e]);
		printf("\t|\n");
	}
}

int main ( int argc, char *argv[]){
	int *mA, *mB, *mC, segmentoA, segmentoB, segmentoC, id, i, e;
	// aloca a memória compartilhada
	segmentoA = shmget (IPC_PRIVATE, M_SIZE*sizeof(int), IPC_CREAT | IPC_EXCL | S_IRUSR |S_IWUSR);// associa a memória compartilhada ao processo
	mA = (int*) shmat (segmentoA, 0, 0);// comparar o retorno com -1

	mA[0] = 1; mA[1] = 2; mA[2] = 3;
	mA[3] = 4; mA[4] = 5; mA[5] = 6;
	mA[6] = 7; mA[7] = 8; mA[8] = 9;

	segmentoB = shmget(IPC_PRIVATE, M_SIZE*sizeof(int), IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);
	mB = (int*) shmat(segmentoB, 0, 0);
	
	mB[0] = 1; mB[1] = 0; mB[2] = 1;
	mB[3] = 0; mB[4] = 2; mB[5] = 0;
	mB[6] = 1; mB[7] = 1; mB[8] = 3;

	segmentoC = shmget(IPC_PRIVATE, M_SIZE*sizeof(int), IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);
	mC = (int*) shmat(segmentoC, 0, 0);
	
	mC[0] = 0; mC[1] = 0; mC[2] = 0;
	mC[3] = 0; mC[4] = 0; mC[5] = 0;
	mC[6] = 0; mC[7] = 0; mC[8] = 0;
	
	for(i=0;i<N_FILHO;i++){
		id = 0;
		id = fork();
		if(id<0){
			puts ("Erro na criação do novo processo\n");
			exit (-2);
		}
		else if(id==0) break;
	}

	if(id == 0){
//		mA[3] = mA[3]+10;
		for(e=0;e<M_LINHA;e++)
			mC[e+i*M_LINHA] = mA[e+i*M_LINHA] + mB[e+i*M_LINHA];
		printf ("Processo %d filho\n", getpid());
		exit(1);
	}
	else{
		waitpid(id, NULL, 0);
	}
	
	printf ("p %d - matriz A:\n", getpid());
	exibeMatriz(mA, M_LINHA, M_LINHA);
	printf ("p %d - matriz B:\n", getpid());
	exibeMatriz(mB, M_LINHA, M_LINHA);
	printf ("p %d - matriz somadas:\n", getpid());
	//exibeMatriz(mC, M_LINHA, M_LINHA);
	for(i=0;i<M_LINHA;i++){
		printf ("p - %d: %d\t%d\t%d\n", getpid(), mC[i*M_LINHA], mC[i*M_LINHA+1], mC[i*M_LINHA+2]);
	}
	printf ("Processo pai %d\n", getpid());
	

	// libera a memória compartilhada do processo
	shmdt(mA);
	shmdt(mB);
	shmdt(mC);
	// libera a memória compartilhada
	shmctl(segmentoA, IPC_RMID, 0);
	shmctl(segmentoB, IPC_RMID, 0);
	shmctl(segmentoC, IPC_RMID, 0);
	return 0;
}
-----------------------------------------------------
COMPILAÇÃO E EXECUÇÃO
$ gcc -o prog matriz.c
$ ./prog
-----------------------------------------------------
RESULTADO
Processo 3319 filho
Processo 3318 filho
Processo 3320 filho
p 3317 - matriz A:
|	1	2	3	|
|	4	5	6	|
|	7	8	9	|
p 3317 - matriz B:
|	1	0	1	|
|	0	2	0	|
|	1	1	3	|
p 3317 - matriz somadas:
p - 3317: 2	2	4
p - 3317: 4	7	6
p - 3317: 8	9	12
Processo pai 3317

-----------------------------------------------------
CONCLUSÃO

	Usando as funções shmget() alocamos uma área de memória. E com shmat, usamos o id dessa área de memória para passa o endereço dela para um ponteiro. Dai me diante podemos tratar esse ponteiro, como qualquer variável que já conhecemos, mas sem esquecer de liberar no final. O processo é bem semelhante a uma alocação dinâmica com malloc().
	A diferença é que ao criar um novo processo, a memória usada é compartilhada ao invés de copiada, então mais de um processor pode alterar a mesma variável ao mesmo tempo, refletindo em outros processos.

-----------------------------------------------------
QUESTÃO 2) Faça um programa que:
	Leia a mensagem do dia do teclado
	Crie uma memória compartilhada com a chave 8180
	Salve a mensagem na memória
	Faça um outro programa que utilize a mesma chave (8180) e exiba a mensagem do dia para o usuário.
-----------------------------------------------------
ARQUIVO.C

//msgEscreve.c

#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/wait.h>

#define MENSAGEM_MAX 80
#define CHAVE 8180

int main ( int argc, char *argv[]){
	int segmento, i;
	char *mensagem;
	// aloca a memória compartilhada
	segmento = shmget (CHAVE, MENSAGEM_MAX*sizeof(char), IPC_CREAT |  S_IRUSR |S_IWUSR);// associa a memória compartilhada ao processo
	mensagem = (char*) shmat (segmento, 0, 0);// comparar o retorno com -1

	if(segmento<0)printf("erro\n");

	scanf(" %[^\n]", mensagem);
	printf("%s\n", mensagem);
	for(i=30;i>5;i--){
		printf("%d\n", i);
		sleep(1);
	}
	printf("SEU TEMPO ESTÁ ACABANDO!\n");
	for(i=5;i>0;i--){
		printf("%d\n", i);
		sleep(1);
	}
	printf("VAI EXPLODIR!!\n");
	printf("%s\n", mensagem);

	// libera a memória compartilhada do processo
	shmdt(mensagem);
	// libera a memória compartilhada
	//shmctl(segmento, IPC_RMID, 0);
	return 0;
}

//msgLe.c

#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/wait.h>

#define MENSAGEM_MAX 80
#define CHAVE 8180

int main ( int argc, char *argv[]){
	int segmento, ;
	char *mensagem;
	// aloca a memória compartilhada
	segmento = shmget (CHAVE, MENSAGEM_MAX*sizeof(char), IPC_EXCL | S_IRUSR |S_IWUSR);// associa a memória compartilhada ao processo
	mensagem = (char*) shmat (segmento, 0, 0);// comparar o retorno com -1

	printf("mensagem de outro programa:\n%s\n", mensagem);

	// libera a memória compartilhada do processo
	shmdt(mensagem);
	// libera a memória compartilhada
	shmctl(segmento, IPC_RMID, 0);
	return 0;
}
-----------------------------------------------------
COMPILAÇÃO E EXECUÇÃO
$ gcc -o progA msgEscreve.c
$ gcc -o progB msgLe.c
$ ./progA
$ ./progB
-----------------------------------------------------
RESULTADO
I've got the power
I've got the power
30
29
28
27
26
25
24
23
22
21
20
19
18
17
16
15
14
13
12
11
10
9
8
7
6
SEU TEMPO ESTÁ ACABANDO!
5
4
3
2
1
VAI EXPLODIR!!
I've got the power

[c1613172@jurua aula6]$ ./progB
mensagem de outro programa:
I've got the power

-----------------------------------------------------
CONCLUSÃO
	Se ao final do programa a memória não for liberada, ela continua alocada, possibilitando que outro programa que conheça aquele id acesse a memória.
	Isso também traz grandes possibilidades para vazamento de memória caso ela não seja liberada, pois ela não será mesmo com o encerramento do programa.
	Aqui foi essencial mudar uma das flags, em msgLe, para que a area de memória ja existente não seja sobreescrita ou retorne erro.
	A princípio eu acreditei que fosse necessário manter o primeiro programa em execução para que o segundo acessasse a memória, por isso implementei a contagem regressiva, que possibilita o teste das duas situações.

QUESTÃO 3) Faça um programa paralelo para achar a
transposta de uma matriz.
	Crie uma matriz na memória compartilhada e leia
dados do teclado para preencher a matriz (4x4)
	Crie outra matriz para armazenar a matriz
transposta. Gere 4 processos para calcular a
transposta onde cada processo deve transpor
uma linha para a respectiva coluna da matriz
transposta.
	O processo pai deve imprimir a matriz original e a
transposta.
-----------------------------------------------------
ARQUIVO.C
#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/wait.h>

#define M_SIZE 16
#define M_LINHA 4
#define N_FILHO 4

void exibeMatriz(int *m, int linhas, int colunas){
	int i, e;
	for(i=0;i<linhas;i++){
		printf("|");
		for(e=0;e<colunas;e++)
			printf("\t%d", m[i*linhas+e]);
		printf("\t|\n");
	}
}


int main ( int argc, char *argv[]){
	int *mA, *mB, segmentoA, segmentoB, id, i, e;
	// aloca a memória compartilhada
	segmentoA = shmget (IPC_PRIVATE, M_SIZE*sizeof(int), IPC_CREAT | IPC_EXCL | S_IRUSR |S_IWUSR);// associa a memória compartilhada ao processo
	mA = (int*) shmat (segmentoA, 0, 0);// comparar o retorno com -1
	
	for(i=0;i<M_SIZE;i++){
		printf("Digite o valor [%d][%d] da matriz: ", i/M_LINHA, i%M_LINHA);
		scanf("%d", mA+i);
	}
	exibeMatriz(mA, M_LINHA, M_LINHA);

	segmentoB = shmget(IPC_PRIVATE, M_SIZE*sizeof(int), IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);
	mB = (int*) shmat(segmentoB, 0, 0);
	

	for (i=0;i<N_FILHO;i++){
		id=0;
		id=fork();
		if(id<0){
			printf("error\n");
			exit(1);
		} 
		else if(id==0) break;
	}
	if(id==0){
		for(e=0;e<M_LINHA;e++){
			printf("f %d: mA[%d][%d]=%d --> mB[%d][%d]\n",  getpid(), i, e, mA[i*M_LINHA+e], e, i);
			mB[e*M_LINHA+i] = mA[i*M_LINHA+e];
		}
		exit(0);
	}
	
	waitpid(id, NULL, 0);

	printf("p %d - matriz original:\n", getpid());
	exibeMatriz(mA, M_LINHA, M_LINHA);
	printf("p %d - matriz transposta:\n", getpid());
	exibeMatriz(mB, M_LINHA, M_LINHA);

	// libera a memória compartilhada do processo
	shmdt(mA);
	shmdt(mB);
	// libera a memória compartilhada
	shmctl(segmentoA, IPC_RMID, 0);
	shmctl(segmentoB, IPC_RMID, 0);
	return 0;
}
-----------------------------------------------------
COMPILAÇÃO E EXECUÇÃO
$ gcc -o prog transposta.c
$ ./prog
-----------------------------------------------------
RESULTADO
Digite o valor [0][0] da matriz: 1
Digite o valor [0][1] da matriz: 2
Digite o valor [0][2] da matriz: 3
Digite o valor [0][3] da matriz: 4
Digite o valor [1][0] da matriz: 5
Digite o valor [1][1] da matriz: 6
Digite o valor [1][2] da matriz: 7
Digite o valor [1][3] da matriz: 8
Digite o valor [2][0] da matriz: 9
Digite o valor [2][1] da matriz: 0
Digite o valor [2][2] da matriz: 1
Digite o valor [2][3] da matriz: 2
Digite o valor [3][0] da matriz: 3
Digite o valor [3][1] da matriz: 4
Digite o valor [3][2] da matriz: 5
Digite o valor [3][3] da matriz: 6
|	1	2	3	4	|
|	5	6	7	8	|
|	9	0	1	2	|
|	3	4	5	6	|
f 3666: mA[0][0]=1 --> mB[0][0]
f 3666: mA[0][1]=2 --> mB[1][0]
f 3666: mA[0][2]=3 --> mB[2][0]
f 3666: mA[0][3]=4 --> mB[3][0]
f 3668: mA[2][0]=9 --> mB[0][2]
f 3668: mA[2][1]=0 --> mB[1][2]
f 3668: mA[2][2]=1 --> mB[2][2]
f 3668: mA[2][3]=2 --> mB[3][2]
f 3667: mA[1][0]=5 --> mB[0][1]
f 3667: mA[1][1]=6 --> mB[1][1]
f 3667: mA[1][2]=7 --> mB[2][1]
f 3667: mA[1][3]=8 --> mB[3][1]
f 3669: mA[3][0]=3 --> mB[0][3]
f 3669: mA[3][1]=4 --> mB[1][3]
f 3669: mA[3][2]=5 --> mB[2][3]
f 3669: mA[3][3]=6 --> mB[3][3]
p 3663 - matriz original:
|	1	2	3	4	|
|	5	6	7	8	|
|	9	0	1	2	|
|	3	4	5	6	|
p 3663 - matriz transposta:
|	1	5	9	3	|
|	2	6	0	4	|
|	3	7	1	5	|
|	4	8	2	6	|

-----------------------------------------------------
CONCLUSÃO
	Bastante semelhante ao primeiro exercício. Filhos são criados de acordo com o número de linhas na matriz, a própria variável usada no loop para gerá-los irá dizer qual linha/coluna aquele filho deve trabalhar. Os filhos terminam a execução assim quer terminam sua tarefa e liberamos o a memória ao final do programa.

