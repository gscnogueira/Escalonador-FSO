#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include "processosAuxiliares.c"

#define n_auxiliares 4 
int *memC;

void Escalonador(){
	FILE *arquivo;
	char linha[10];
	int qtdLinhas, status;
	pid_t pidAux;
	
	// escrita memC
	arquivo = fopen("processos.txt", "rt");
	int i = 0;
	printf("%ld\n", sizeof(arquivo));
	while(fgets(linha, sizeof(linha), arquivo) != NULL){
		memC[i] = strtol(linha, NULL, 10); //converte pra int
		i++;
		qtdLinhas++;
	}
	fclose(arquivo);
	
	
	
	//cria auxiliares
	for(int ij = 0; ij < n_auxiliares; ij++){
		//printf("memC[%d]: %d\n", 0, memC[0]);
		
		pidAux = fork();
		if(pidAux == 0){
			auxiliares(ij, memC);
			break;
		}
	}
	
	//espera filhos
	if(pidAux > 0){
		for(int ik = 0; ik < n_auxiliares; ik++){
			wait(&status);
		}
	}
}

void main(int argcm,char **agrv){

	// memória compartilhada
	int sid = shmget(IPC_PRIVATE, sizeof(int)*10, SHM_R|SHM_W|IPC_CREAT);
	memC = (int *) shmat(sid, NULL, 0);
	
	pid_t pidEscalonador = fork();
	
	if(pidEscalonador == 0){
		Escalonador();
		printf("auxiliares finalizaram\n");
	}
	 
}
