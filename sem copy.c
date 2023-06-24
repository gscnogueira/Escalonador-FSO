#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/wait.h>


struct sembuf operacao [2];
int idsem;
int quem=0;
int semval;

void p_sem(){

    semval = semctl(idsem, 0, GETVAL);         // Obtenha o valor do semáforo

    operacao[0].sem_num = 0;
    operacao[0].sem_op = 0;
    operacao[0].sem_flg = 0;

    operacao[1].sem_num = 0;
    operacao[1].sem_op = 1;
    operacao[1].sem_flg = 0;
    if (semop(idsem, operacao, 2) < 0)
            printf("erro no p=%d\n", errno);

    semval = semctl(idsem, 0, GETVAL);         // Obtenha o valor do semáforo
}

void v_sem(){
    operacao[0].sem_num = 0;
    operacao[0].sem_op = -1;
    operacao[0].sem_flg = 0;
    if (semop(idsem, operacao, 1) < 0)
            printf("erro no p=%d\n", errno);
}

int main(){
    int pid, estado;
    int *psem;
    if((idsem = semget(0x1223, 1, IPC_CREAT | 0x1ff)) < 0){
        printf("erro na criação do semáforo\n");
        exit(1);
    }

    if ((pid = fork()) == 0){
        /*código do filho*/
        quem = 1;
        printf("%d:to parado\n", quem);
        p_sem();
        sleep(5);
        printf("filho - obtive o semáforo\n");
        printf("filho - vou liberar o semáforo\n");
        v_sem();
        exit(0);
    }

    /*código do pai*/
    printf("%d:to parado\n", quem);
    p_sem();
    printf("pai - obtive o semáforo\n");
    printf("pai - vou liberar o semáforo\n");
    v_sem();

    wait(&estado);
    printf("%d: Valor atual do semáforo: %d\n", quem, semval);
    return 0;
}