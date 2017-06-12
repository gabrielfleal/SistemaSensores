#include<semaphore.h>
#include<pthread.h>
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>

#define N 5
#define MAX_COLETAS 20

int i, int_rand, x, y, z;
int tamanhoFila, nrExames;
float float_rand;
int matrizDados[4][2];  //Primeira coluna-> id do sensor; Segunda coluna -> dado do sensor; Duas primeiras linhas -> Conjunto A; Duas �ltimas linhas -> Conjunto B

void gerarDado(int);
void adicionaDadoMatriz(int, int);
void coletar(int, int);
void printaMatriz();
void liberaMatriz();
void *acao_sensor(void *);

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

void printaMatriz(){
    int cont;
        printf("\n\n--------- Coleta -----------");
    for(cont=0; cont<4; cont++){
        printf("\nO sensor de id %d coletou o dado %d", matrizDados[cont][0], matrizDados[cont][1]);
    }
}

void liberaMatriz(){
    for(y = 0; y < 4; y++){
        matrizDados[y][0] = -1;
        matrizDados[y][1] = -1;
    }
}

int idPresente(int id){
    for(z=0; z<4; z++){
        if(matrizDados[z][0]==id){
            return 0;
        }
    }
    return 1;
}

void *acao_sensor(void *j){
  int i = *(int*) j;
	while(tamanhoFila<MAX_COLETAS){
    sleep(1);
		gerarDado(i);
	}
}

void gerarDado(int i){
	int_rand=(random()%150);
	coletar(int_rand, i);
}

void monitor(){
  	Node *nodoA=(Node *) malloc(sizeof(Node));
		Node *nodoB=(Node *) malloc(sizeof(Node));

		nodoA->proximo=NULL;
		nodoB->proximo=NULL;

    nodoA->conjuntoDados[0][0] = matrizDados[0][0];
    nodoA->conjuntoDados[0][1] = matrizDados[0][1];
    nodoA->conjuntoDados[1][0] = matrizDados[1][0];
    nodoA->conjuntoDados[1][1] = matrizDados[1][1];
		insereNodoFila(nodoA);

    nodoB->conjuntoDados[0][0] = matrizDados[2][0];
    nodoB->conjuntoDados[0][1] = matrizDados[2][1];
    nodoB->conjuntoDados[1][0] = matrizDados[3][0];
    nodoB->conjuntoDados[1][1] = matrizDados[3][1];
		insereNodoFila(nodoB);

    liberaMatriz();
}

void coletar(int dado, int id){
    for(x=0; x<4; x++){
        pthread_mutex_lock(&mutex);
        if(matrizDados[x][0]==-1){
            if(idPresente(id)){
                matrizDados[x][0]=id;
                matrizDados[x][1]=dado;
                if(x==3){
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

void visualizar(){
	int dadosSensores[5][MAX_COLETAS];

	Node *tmp;
	tmp = fila->proximo;
	int countDados[5]={0, 0, 0, 0, 0};

	while( tmp != NULL){
		for(y=0; y<2; y++){
			switch (tmp->conjuntoDados[y][0]) {
				case 0:
					dadosSensores[0][countDados[0]]=tmp->conjuntoDados[y][1];
					countDados[0]++;
					break;
				case 1:
					dadosSensores[1][countDados[1]]=tmp->conjuntoDados[y][1];
					countDados[1]++;
					break;
				case 2:
					dadosSensores[2][countDados[2]]=tmp->conjuntoDados[y][1];
					countDados[2]++;
					break;
				case 3:
					dadosSensores[3][countDados[3]]=tmp->conjuntoDados[y][1];
					countDados[3]++;
					break;
				case 4:
					dadosSensores[4][countDados[4]]=tmp->conjuntoDados[y][1];
					countDados[4]++;
					break;
			}
		}
		tmp = tmp->proximo;
	}
	double media;
 	double acmDados;
	for(y=0; y<N; y++){
		acmDados=0;
		for(z=0; z<countDados[y]; z++){
			acmDados+=dadosSensores[y][z];
		}
		media = acmDados/countDados[y];
		printf("\n\n------------------------ Sensor ID %d ------------------------", y);
		switch (y) {
			case 0:
				printf("\nA média coletada pelo sensor de ritmo cardíaco é: %.2f", media);
				break;
			case 1:
				printf("\nA média coletada pelo sensor de suprimento sanguíneo é: %.2f", media);
				break;
			case 2:
				printf("\nA média coletada pelo sensor de suprimento de oxigênio é: %.2f", media);
				break;
			case 3:
				printf("\nA média coletada pelo sensor de despolarização atrial é: %.2f", media);
				break;
			case 4:
				printf("\nA média coletada pelo sensor de repolarização ventricular é: %.2f", media);
				break;
		}
		printf("\nA quantidade de vezes que foi escolhido: %d", countDados[y]);
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
