#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <string.h>
#include <string.h>
#include <stdio.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#define TAM_WORD  10
#define TAM_LINHA 100
#define N_AUX 4 // número de processos auxiliares

int *memC;

void print_error(char* error_msg){
    printf("\033[31mErro:\033[0m");
    printf("%s\n", error_msg);
}

int plexc(int *memC, int p_aux_id, int n){


    int state;
    char path[] = "./processes/";

    for(size_t i = p_aux_id; i < n; i+= N_AUX) {
        printf("Aux %d:Executando %c\n",p_aux_id, memC[i]);
        pid_t pid = fork();

        if (pid < 0){
            print_error("Não foi possível criar o processo");
            return -1;
        }

        if(pid == 0){
            if (memC[i] == 'f') {
                strcat(path, "fast");
                execl(path, "fast", NULL);
            }
            else if (memC[i] == 'm') {
                strcat(path, "medium");
                execl(path, "medium", NULL);
            }
            else if (memC[i] == 's') {
                strcat(path, "slow");
                execl(path, "slow", NULL);
            }

            print_error("Não foi possível executar o processo");
            return -1;
        }

        wait(&state);
    }
}

// int plexcws(process_list_t* plist, int p_aux_id){
//     // Função para executar work stealing
//     return 0;
// }


void preproc_line(char* line){
    size_t len = strlen(line);

    if (len > 0 && line[len-1] == '\n')
        line[len-1] = '\0';
}

int main(int argc, char *argv[]){

    enum mode_t {NORMAL, WORK_STEALING};
    mode_t mode;
    int estado, pid, line_number=0, p_aux_id=0; 
    char line[TAM_LINHA];
    // process_list_t plists[N_AUX];
    FILE *input_file;
    struct timeval stop, start;



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


    // inicializa as listas de processos
    for(int i = 0; i < N_AUX; i++)
    
    while (fgets(line, sizeof(line), input_file) != NULL) line_number++;

    int sid = shmget(IPC_PRIVATE, line_number * sizeof(char), SHM_R|SHM_W|IPC_CREAT);
    memC = (int *) shmat(sid, NULL, 0);
    
    rewind(input_file);
    line_number = 0;
    
    while (fgets(line, sizeof(line), input_file) != NULL){
        memC[line_number++] = line[0];
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
        printf("Tempo de turnaround: %lf segundos\n", ((stop.tv_sec - start.tv_sec)*1e6 + (stop.tv_usec - start.tv_usec))/1e6);
    }
    else if (mode == NORMAL) {
        // plexc(plists + p_aux_id, p_aux_id);
        plexc(memC, p_aux_id, line_number);
    }
    /*
    else if (mode == WORK_STEALING){
        // plexcws(plists + p_aux_id, p_aux_id);
        plexcws(plists + p_aux_id, p_aux_id);
    }
    */

    return 0;
}
