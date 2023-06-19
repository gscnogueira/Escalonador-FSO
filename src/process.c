#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "process.h"
#include "error.h"
#include <sys/wait.h>


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

void plprint(process_list_t* plist){

    process_t* current = plist->front;

    printf("------\n");
    printf("Lista contém %d elementos\n", plist->size);
    while(current){
        printf("%s", current->name);
        current=current->next;
    }
}

int plexc(process_list_t* plist, int p_aux_id){
    process_t* current = plist->front;

    while(current){

        pexc(current, p_aux_id);
        current=current->next;
    }

    return 0;
}

int pexc(process_t* p, int p_aux_id){

    int state;
    char path[] = "./processes/";


    printf("Aux %d:Executando %s\n",p_aux_id, p->name);
    pid_t pid = fork();

    if (pid < 0){
        print_error("Não foi possível criar o processo");
        return -1;
    }

    if(pid == 0){
        strcat(path, p->name);
        execl(path, p->name, NULL);
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


