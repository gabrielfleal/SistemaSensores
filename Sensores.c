#include<semaphore.h>
#include<pthread.h>
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>

#define N 5
#define LIN 4
#define COL 2

#define MAX_COLETAS 20

int i, int_rand, x, y, z;
int tamanhoFila, nrExames;
float float_rand;
int matrizDados[LIN][COL];  //Primeira coluna-> id do sensor; Segunda coluna -> dado do sensor; Duas primeiras linhas -> Conjunto A; Duas �ltimas linhas -> Conjunto B

pthread_mutex_t mutex;
sem_t sem_sensores[N];

struct Node{
	int conjuntoDados[2][2];
	struct Node *proximo;
};
typedef struct Node Node;

Node *fila;
int tamanhoFila, nodoAtual;

void inicializaFila(Node *fila) {
	fila->proximo = NULL;
	tamanhoFila=0;
  nodoAtual=0;
}
int filaVazia(){
    if(fila->proximo == NULL){
        return 1;
    } else{
        return 0;
    }
}
void liberaFila(){
	if(!filaVazia()){
		Node *proximoNodo, *atual;
		atual = fila->proximo;
		while(atual != NULL){
			proximoNodo = atual->proximo;
			free(atual);
			atual = proximoNodo;
		}
	}
	tamanhoFila=0;
}
void insereNodoFila(Node *novoNodo){
		if(filaVazia()){
			fila->proximo=novoNodo;
		}else{
			Node *tmp = fila->proximo;
			while(tmp->proximo != NULL){
				tmp = tmp->proximo;
			}
			tmp->proximo = novoNodo;
		}
		tamanhoFila++;
}

void *acao_sensor(void *j){
  int i = *(int*) j;
  while(tamanhoFila<MAX_COLETAS){
    sleep(1);
    gerarDado(i);
  }

}

void liberaMatriz(){
    for(y = 0; y < 4; y++){
       	matrizDados[y][0] = -1;
      	matrizDados[y][1] = -1;
    }
}

void monitor(){
    Node *nodoA=(Node *) malloc(sizeof(Node));
    Node *nodoB=(Node *) malloc(sizeof(Node));

    nodoA->proximo=NULL;
    nodoB->proximo=NULL;

		for(y=0; y<2; y++){
			for(z=0; z<2; z++){
					nodoA->conjuntoDados[y][z] = matrizDados[y][z];
					nodoB->conjuntoDados[y][z] = matrizDados[y+2][z];
			}
		}
    insereNodoFila(nodoA);
    insereNodoFila(nodoB);

    liberaMatriz();
}

int idPresente(int id){
  for(z=0; z<LIN; z++){
    if(matrizDados[z][0]==id){
        return 0;
    }
  }
  return 1;
}

void coletar(int id){
	 int dado=(random()%150);
    for(x=0; x<4; x++){
        pthread_mutex_lock(&mutex);
        if(matrizDados[x][0]==-1){
            if(idPresente(id)){
                matrizDados[x][0]=id;
                matrizDados[x][1]=dado;
                if(x==COL-1){
                    printaMatriz();
                    monitor();
                    for(z=0; z<N; z++){
                        if(z!=id){
                            sem_post(&sem_sensores[z]);
                        }
                    }
                    pthread_mutex_unlock(&mutex);
                    sleep(3);
                }else{
                    pthread_mutex_unlock(&mutex);
                    sem_wait(&sem_sensores[id]);
                }
            }else{
                pthread_mutex_unlock(&mutex);
                sem_wait(&sem_sensores[id]);
            }
        }else{
            pthread_mutex_unlock(&mutex);
        }
    }
}

void printaMatriz(){
    int cont;
    printf("\n\n--------- Coleta -----------");
    for(cont=0; cont<LIN; cont++){
        printf("\nO sensor de id %d coletou o dado %d", matrizDados[cont][0], matrizDados[cont][1]);
    }
}

int main(){

  int opcao, opcao2;
  fila = (Node *) malloc(sizeof(Node));
  inicializaFila(fila);

  nrExames = 0;
  liberaMatriz();

	pthread_t thread[N];
	void *thread_result;

  pthread_mutex_init(&mutex, NULL);
	for(i=0; i<N; i++)
		sem_init(&sem_sensores[i], 0, 0);

    do{
    	printf("Escolha uma das opções abaixo:\n1 - Iniciar o exame;\n0 - Encerrar o sistema;\n");
    	scanf("%d", &opcao);
  		switch(opcao){
        case 1:
        		printf("Início do Exame\n");
						liberaFila();
            int array[N];
            for(i=0; i<N; i++){
                array[i]=i;
                pthread_create(&thread[i], NULL, acao_sensor, &array[i]);
            }
            for(i=0; i<N; i++)
                pthread_join(thread[i], &thread_result);

            do{
                printf("\n\nExame realizado!\nEscolha uma das opções abaixo:\n1 - Exibir o resultado dos dados coletados;\n0 - Encerrar;\n");
                scanf("%d", &opcao);
                switch(opcao){
                    case 1:
												visualizar();
												break;
                    case 0:
                        printf("\nFim do exame\n");
                        break;
                    default:
                        printf("Erro! Digite novamente");
                        break;
                }
            }while(opcao!=0);
        case 0:
            printf("\nFim da execução.\n");
            break;
        default:
            printf("Erro! Digite novamente");
        }
    } while(opcao!=0);
	return 0;
}
