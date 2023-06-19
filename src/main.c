#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/time.h>
#include "error.h"
#include "process.h"
#include "utils.h"

#define TAM_LINHA 100
#define N_AUX 4 // número de processos auxiliares


int main(int argc, char *argv[]){

    int estado, pid, line_number=0, p_aux_id=0; 
    char line[TAM_LINHA];
    process_list_t plists[N_AUX];
    FILE *input_file = fopen(argv[1], "r");
    struct timeval stop, start;

    if (argc != 2) {
        print_error("Número inválido de argumentos. Forneça apenas um parâmetro");
        return 1;
    }


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
    else {
        plexc(plists + p_aux_id, p_aux_id);
    }

    return 0;
}
