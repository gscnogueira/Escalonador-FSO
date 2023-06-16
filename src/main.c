#include <stdio.h>
#define TAM_LINHA 100


int main(int argc, char *argv[]){

    if (argc != 2) {
        printf("\033[31mErro:\033[0m Número inválido de argumentos. Forneça apenas um parâmetro\n");
        return 1;
    }

    FILE *input_file = fopen(argv[1], "r");

    if (input_file == NULL) {
        printf("\033[31mErro:\033[0m Não foi possível abrir o arquivo\n");
    }

    char line[TAM_LINHA];

    while (fgets(line, sizeof(line), input_file) != NULL){
        printf("%s", line);
    }

    return 0;
}
