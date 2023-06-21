#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>

#define MAX_LINE_LENGTH 80

typedef struct {
  int *array;
  size_t used;
  size_t size;
} Array;

void initArray(Array *a, size_t initialSize) {
  a->array = malloc(initialSize * sizeof(int));
  a->used = 0;
  a->size = initialSize;
}

void insertArray(Array *a, int element) {
  // a->used is the number of used entries, because a->array[a->used++] updates a->used only *after* the array has been accessed.
  // Therefore a->used can go up to a->size 
  if (a->used == a->size) {
    a->size *= 2;
    a->array = realloc(a->array, a->size * sizeof(int));
  }
  a->array[a->used++] = element;
}

void freeArray(Array *a) {
  free(a->array);
  a->array = NULL;
  a->used = a->size = 0;
}

int main() {
   int pid, estado;
   char path[] = "./file.txt";
   char path_processes[] = "./processes/";
   Array aux[4];
   int type_process_now;
   int aux_now;
   int process_now;

   initArray(&aux[0], 1);
   initArray(&aux[1], 1);
   initArray(&aux[2], 1);
   initArray(&aux[3], 1);

   char line[MAX_LINE_LENGTH] = {0};
   unsigned int line_count = 0;

   FILE *file = fopen(path, "r");

    while (fgets(line, MAX_LINE_LENGTH, file))
    {
        line[strcspn(line, "\n")] = 0;

        if (strcmp(line,"slow") == 0) {
         type_process_now = 1;
        }

        if (strcmp(line,"medium") == 0) {
         type_process_now = 2;
        }

        if (strcmp(line,"fast") == 0) {
         type_process_now = 3;
        }

        aux_now = line_count % 4;

        insertArray(&aux[aux_now], type_process_now);

        line_count++;
    }

    time_t start = time(0);

    if (pid = fork() == 0) {
      for (int i = 0; i < aux[0].used; i++) {
         process_now = aux[0].array[i];

         if (pid = fork() == 0) {
            if (process_now == 1) {
               printf("Aux1 executando: slow\n");
               execl("./processes/slow", "./processes/slow", (char*) NULL);
            }

            if (process_now == 2) {
               printf("Aux1 executando: medium\n");
               execl("./processes/medium", "./processes/medium", (char*) NULL);
            }

            if (process_now == 3) {
               printf("Aux1 executando: fast\n");
               execl("./processes/fast", "./processes/fast", (char*) NULL);
            }
         }

         wait(&estado);
      }

      exit(0);
    }

    if (pid = fork() == 0) {
      for (int i = 0; i < aux[1].used; i++) {
         process_now = aux[1].array[i];

         if (pid = fork() == 0) {
            if (process_now == 1) {
               printf("Aux2 executando: slow\n");
               execl("./processes/slow", "./processes/slow", (char*) NULL);
            }

            if (process_now == 2) {
               printf("Aux2 executando: medium\n");
               execl("./processes/medium", "./processes/medium", (char*) NULL);
            }

            if (process_now == 3) {
               printf("Aux2 executando: fast\n");
               execl("./processes/fast", "./processes/fast", (char*) NULL);
            }
         }

         wait(&estado);
      }

      exit(0);
    }

    if (pid = fork() == 0) {
      for (int i = 0; i < aux[2].used; i++) {
         process_now = aux[2].array[i];

         if (pid = fork() == 0) {
            if (process_now == 1) {
               printf("Aux3 executando: slow\n");
               execl("./processes/slow", "./processes/slow", (char*) NULL);
            }

            if (process_now == 2) {
               printf("Aux3 executando: medium\n");
               execl("./processes/medium", "./processes/medium", (char*) NULL);
            }

            if (process_now == 3) {
               printf("Aux3 executando: fast\n");
               execl("./processes/fast", "./processes/fast", (char*) NULL);
            }
         }

         wait(&estado);
      }

      exit(0);
    }

    if (pid = fork() == 0) {
      for (int i = 0; i < aux[3].used; i++) {
         process_now = aux[3].array[i];

         if (pid = fork() == 0) {
            if (process_now == 1) {
               printf("Aux4 executando: slow\n");
               execl("./processes/slow", "./processes/slow", (char*) NULL);
            }

            if (process_now == 2) {
               printf("Aux4 executando: medium\n");
               execl("./processes/medium", "./processes/medium", (char*) NULL);
            }

            if (process_now == 3) {
               printf("Aux4 executando: fast\n");
               execl("./processes/fast", "./processes/fast", (char*) NULL);
            }
         }

         wait(&estado);
      }

      exit(0);
    }

    wait(&estado);
    wait(&estado);
    wait(&estado);
    wait(&estado);

    time_t end = time(0);

   printf("\nMakespan total de: %f segundos.\n", difftime(end, start));

   freeArray(&aux[0]);
   freeArray(&aux[1]);
   freeArray(&aux[2]);
   freeArray(&aux[3]);

   return 0;
}

