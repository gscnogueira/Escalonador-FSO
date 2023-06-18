#include <string.h>
#include <stdio.h>
#include "error.h"


void print_error(char* error_msg){
    printf("\033[31mErro:\033[0m");
    printf("%s\n", error_msg);
}

