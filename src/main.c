#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <string.h>
#include "error.h"
#include "process.h"
#include "utils.h"

#define TAM_LINHA 100
#define N_AUX 4 // número de processos auxiliares


int main(int argc, char *argv[]){

    enum mode_t {NORMAL, WORK_STEALING};
    mode_t mode;
    int estado, pid, line_number=0, p_aux_id=0; 
    char line[TAM_LINHA];
    process_list_t plists[N_AUX];
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
        plinit(plists + i);

    while (fgets(line, sizeof(line), input_file) != NULL){

        preproc_line(line);
        // inicializa processo
        process_t *p = pinit(line);
        // insere o processo na lista de forma striped
        plinsert(plists + (line_number%N_AUX), p);

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
        printf("Tempo de turnaround: %lf segundos\n", ((stop.tv_sec - start.tv_sec)*1e6 + (stop.tv_usec - start.tv_usec))/1e6);
    }
    else if (mode == NORMAL) {
        plexc(plists + p_aux_id, p_aux_id);
    }
    else if (mode == WORK_STEALING){
        plexcws(plists + p_aux_id, p_aux_id);
    }

    return 0;
}
