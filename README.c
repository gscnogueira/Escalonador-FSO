# Escalonador

Trabalho realizado na disciplina **Fundamentos de Sitemas Operacionais**  na **Universidade de Brasília** (UnB) no semestre **2023/1**

## Compilando 

    git clone "https://github.com/gscnogueira/Escalonador-FSO.git"
    cd Escalonador-FSO
    make (se estiver com makefile)

## Executando

    Se não estiver usando o makefile, compile o arquivo main.c (e também slow.c com executável slow, medium.c com medium e fast.c com fast) e a partir daí utilize o executável gerado (abaixo, utilizamos escalonador como executável).

    Uso: ./escalonador <opção> <arquivo>

    O arquivo exemplo com uma lista de processos a executar é o inputfile.c.

    Opções:
    -n, --normal:           Executa escalonador em modo normal.
    -ws, --work-stealing:   Executa escalonador em modo work stealing.
