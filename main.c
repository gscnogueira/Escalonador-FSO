#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/ipc.h>
#include<sys/shm.h>
#include <string.h>

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


void preproc_line(char* line);

void print_error(char *error_msg);

process_t* pinit(char* p_name);

int pexc(process_t* p, int p_aux_id);

void plinit(process_list_t* p_list);

void plinsert(process_list_t* plist, process_t* p);

void plremove(process_list_t* plist);

int plexc(process_list_t* plist, int p_aux_id);

int plexcws(process_list_t* plist, int p_aux_id);

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

    if((idshm = shmget(0x1223, sizeof(int), IPC_CREAT | 0x1ff)) < 0){
		printf("erro na criação de memória compartilhada\n");
        return 1;
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

    for(int i = 0; i < N_AUX; i++){
        printf("Aux %d:",i);
        plprint(plists->data + i);
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

        for(int i = 0; i < N_AUX; i++){
            printf("Aux %d:",i);
            plprint(plists->data + i);
        }

        shmctl(idshm, IPC_RMID, NULL);

        printf("Tempo de turnaround: %lf segundos\n",
               ((stop.tv_sec - start.tv_sec)*1e6
                + (stop.tv_usec - start.tv_usec))/1e6);
    }
    else {
        if (mode == NORMAL) 
            plexc(plists->data + p_aux_id, p_aux_id);

        if (mode == WORK_STEALING)
            plexcws(plists->data + p_aux_id, p_aux_id);

        shmdt(plists); // detach da memória compartilhada
    }

    return 0;
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

    printf("Aux %d:Executando %s\n",p_aux_id, p->name);
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


    wait(&state);

    return 0;

}

int plexcws(process_list_t* plist, int p_aux_id){
    // Função para executar work stealing
    return 0;
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

