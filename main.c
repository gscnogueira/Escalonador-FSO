#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>

#define TAM_WORD  10
#define TAM_LINHA 100
#define N_AUX 4 // número de processos auxiliares

typedef struct process_t {
    char name [TAM_WORD];
    struct process_t* next;
} process_t;

typedef struct process_list_t {
    unsigned int size;
    process_t* front;
    process_t* back;
} process_list_t;

typedef struct {
    process_list_t data[N_AUX];
} process_list_array_t;


struct sembuf operacao [2];
int idsem;

void p_sem(int sem_num);

void v_sem(int sem_num);

process_t steal(process_list_array_t* plists, int p_aux_id);

void preproc_line(char* line);

void print_error(char *error_msg);

process_t* pinit(char* p_name);

int pexc(process_t* p, int p_aux_id);

void plinit(process_list_t* p_list);

void plinsert(process_list_t* plist, process_t* p);

void plremove(process_list_t* plist);

int plexc(process_list_t* plist, int p_aux_id);

int plexcws(process_list_array_t* plists, int p_aux_id);

void plprint(process_list_t* plist);


int main(int argc, char *argv[]){

    enum mode_t {NORMAL, WORK_STEALING};
    mode_t mode;
    int estado,
        idshm,
        pid,
        line_number=0,
        p_aux_id=0; 
    char line[TAM_LINHA];
    FILE *input_file;
    struct timeval stop, start;
    process_list_array_t* plists;

    if (argc < 3 ) {
        printf("Uso: %s <opção> <arquivo>\n\n", argv[0]);
        printf("Opções:\n");
        printf("-n, --normal:\t\tExecuta escalonador em modo normal.\n");
        printf("-ws, --work-stealing:\tExecuta escalonador em modo work stealing.\n");
        return 1;
    }
    else if (strcmp(argv[1], "-n") == 0 || strcmp(argv[1], "--normal") == 0) {
        mode = NORMAL;
    }
    else if (strcmp(argv[1], "-ws") == 0 || strcmp(argv[1], "--work-stealing") == 0) {
        mode = WORK_STEALING;
    }
    else {
        printf("Opção inválida.\n");
        return -1;
    }

    input_file = fopen(argv[2], "r");

    if (input_file == NULL) {
        print_error("Não foi possível abrir o arquivo");
        return 1;
    }

    if((idshm = shmget(0x1223, sizeof(process_list_array_t), IPC_CREAT | 0x1ff)) < 0){
		printf("erro na criação de memória compartilhada\n\n");
        return 1;
	}

    if((idsem = semget(0x1223, N_AUX , IPC_CREAT | 0x1ff)) < 0){
        printf("erro na criação do semáforo\n");
        exit(1);
    }

    plists = (process_list_array_t*) shmat(idshm, NULL, 0);

    // inicializa as listas de processos
    for(int i = 0; i < N_AUX; i++)
        plinit(plists->data + i);


    while (fgets(line, sizeof(line), input_file) != NULL){

        preproc_line(line);
        // inicializa processo
        process_t *p = pinit(line);
        // insere o processo na lista de forma striped
        plinsert(plists->data + (line_number%N_AUX), p);
        line_number++;

    }
    gettimeofday(&start, NULL);
    for (int i = 0; i < N_AUX; i++){

        pid = fork();

        if (pid < 0){
            print_error("Não foi possível criar o processo");
            return 1;
        }

        if (pid == 0)
            break;

        p_aux_id++;
    }

    // se for processo pai
    if (pid > 0){
        // espera os processos filhos acabarem
        for (int i = 0; i < N_AUX; i++){
            wait(&estado);
        }
        gettimeofday(&stop, NULL);

        // remove memória compartilhada
        if (shmctl(idshm, IPC_RMID, NULL) < 0){
            printf("Erro na remoção da memória compartilhada\n");
            return 1;
        }

        // remove conjunto de semáforos

        if (semctl(idsem, 0, IPC_RMID) < 0){
            printf("Erro na remoção do conjunto de semáforos\n");
            return 1;
        }

        printf("Tempo de turnaround: %lf segundos\n",
               ((stop.tv_sec - start.tv_sec)*1e6
                + (stop.tv_usec - start.tv_usec))/1e6);
    }
    else {
        if (mode == NORMAL) 
            plexc(plists->data + p_aux_id, p_aux_id);

        if (mode == WORK_STEALING){
            plexcws(plists,  p_aux_id);
        }

        shmdt(plists); // detach da memória compartilhada
    }

    return 0;
}

void p_sem(int sem_num){

    operacao[0].sem_num = sem_num;
    operacao[0].sem_op = 0;
    operacao[0].sem_flg = 0;

    operacao[1].sem_num = sem_num;
    operacao[1].sem_op = 1;
    operacao[1].sem_flg = 0;

    if (semop(idsem, operacao, 2) < 0)
            printf("erro no p=%d\n", errno);
}

void v_sem(int sem_num){
    operacao[0].sem_num = sem_num;
    operacao[0].sem_op = -1;
    operacao[0].sem_flg = 0;
    if (semop(idsem, operacao, 1) < 0)
            printf("erro no p=%d\n", errno);
}

process_t* pinit(char* p_name){

    process_t* p = malloc(sizeof(process_t));

    if(!p)
        return p;

    strcpy(p->name, p_name);
    p->next = NULL;

    return p;
}

void plinit(process_list_t* p_list){

    p_list->size = 0;
    p_list->front = NULL;
    p_list->back = NULL;

}

void plinsert(process_list_t* plist, process_t* p){

    // retorna se a lista ou o elemento não existirem
    if (!plist || !p)
        return;

    if(plist->size == 0) {
        plist->front = p;
    }
    else{
        plist->back->next = p;
    }

    plist->back = p;
    plist->size++;
}

void plremove(process_list_t* plist){

    process_t* p = NULL;
    if (!plist || plist->size == 0)
        return;

    p = plist->front;
    plist->front = p->next;

    if(!plist->front)
        plist->back = NULL;

    plist->size--;

    free(p);
}

void plprint(process_list_t* plist){

    process_t* current = plist->front;

    printf("Lista contém %d elementos\n", plist->size);
    while(current){
        printf("%s ", current->name);
        current=current->next;
    }
    printf("\n");
}

int plexc(process_list_t* plist, int p_aux_id){

    while(plist->front){
        pexc(plist->front, p_aux_id);
        plremove(plist);
    }

    return 0;
}

int pexc(process_t* p, int p_aux_id){

    int state;

    pid_t pid = fork();

    if (pid < 0){
        print_error("Não foi possível criar o processo");
        return -1;
    }

    if(pid == 0){
        execl(p->name, p->name, NULL);
        print_error("Não foi possível executar o processo");
        return -1;
    }

    printf("Aux %d (%d): Executando %s\n", p_aux_id, pid,p->name);

    wait(&state);

    return 0;

}

int plexcws(process_list_array_t* plists, int p_aux_id){

    // função para executar work stealing

    int cont = 0;
    process_list_t* plist = plists->data +p_aux_id;
    while(1){
        // checa se a lista do processo está vazia
        // se não, pega o processo da lista, remove e executa
        // se sim, segue para o roubo de processos
        p_sem(p_aux_id);

        if(!plist->front) {
            v_sem(p_aux_id);
            break;
        }

        process_t p = *(plist->front);

        plremove(plist);
        v_sem(p_aux_id);
        pexc(&p, p_aux_id);
        cont++;
    }

    printf("\nAux %d terminou sua fila! (Executou %d processos)\n\n", p_aux_id, cont);

    process_t pchosen;
    
    while (1) {
        // tenta roubar processo até um processo vazio ser retornado
        pchosen = steal(plists, p_aux_id);

        if(strcmp(pchosen.name, "empty") == 0) break;
        pexc(&pchosen, p_aux_id);
    }

    return 0;
}

int choose_list(int limit) {
    // gera um índice aleatório

    time_t t;
    
    srand((unsigned) time(&t));
    
    return rand() % limit;
}

process_t steal(process_list_array_t* plists, int p_aux_id){
    // rouba e retorna um processo, sem executá-lo

    int tam, id_fila;
    int fila_ids[] = {0, 1, 2, 3};
    int found = 0;
    int cont = 0;
    process_t p;
    int limit = N_AUX;

    while (limit){
        // checa todas as listas para ver se há processo para ser roubado
        id_fila = fila_ids[choose_list(limit)];
        cont++;

        p_sem(id_fila);

        tam = (plists->data + id_fila)->size;

        if (tam){
            // se tem processo para ser roubado (lista com tamanho maior que 0), não dá v_sem e segue para a remoção do processo
            found = 1;
            break;
        }
        v_sem(id_fila);

        int aux = fila_ids[limit-1];
        fila_ids[limit-1] = fila_ids[id_fila];
        fila_ids[id_fila] = aux;
        limit--;
    }

    if (!found) {
        // se não tem processo para ser roubado, apenas retorna
        strcpy(p.name, "empty");
        return p;
    }

    printf("Aux %d roubando da fila de Aux %d!\n", p_aux_id, id_fila);
    process_list_t *plist = plists-> data + id_fila;
    p = *(plist->front);
    plremove(plist);

    // com o processo já removido, libera o semáforo da lista
    v_sem(id_fila);

    return p;
}

void print_error(char* error_msg){
    printf("\033[31mErro:\033[0m");
    printf("%s\n", error_msg);
}

void preproc_line(char* line){
    size_t len = strlen(line);

    if (len > 0 && line[len-1] == '\n')
        line[len-1] = '\0';
}

